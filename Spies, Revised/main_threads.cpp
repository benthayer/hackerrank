#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <thread>
#include <mutex>
#include <utility>
#include <chrono>

using namespace std;
using namespace std::chrono;

class Solver;

class SolutionMaster {
    int n;
    int nThreads;
    mutex m;

    vector<Solver> solvers;
    int nextSolver = 0;
    vector<thread> active;
    vector<int> solution;

    public:
    int maxDepth = 0;

    SolutionMaster(int n, int t);
    void enqueue(Solver* s);
    void markSolved(Solver* s);
    void startThread();
    vector<int> getSolution();
};

class Solver {
    public:
    int n;

    bool** spotAvailable;
    vector<pair<int, int>>* invalidations;
    int* freeSpaceInColumns;
    bool* columnsLeft;

    // solution information
    int* layerColumns;
    int* layerRows;
    int currentLayer;

    vector<pair<int, int>> slopes;

    SolutionMaster* master;
    bool solutionFound = false;

    Solver(int n, SolutionMaster* m):n(n),master(m) {

        spotAvailable = new bool*[n];
        for (int i = 0; i < n; i++) {
            spotAvailable[i] = new bool[n];
            for (int j = 0; j < n; j++) {
                spotAvailable[i][j] = true;
            }
        }
        

        invalidations = new vector<pair<int, int>>[n]; // spots layer has invalidated
        for (int i = 0; i < n; i++) {
            invalidations[i].reserve(n*5);
        }

        freeSpaceInColumns = new int[n];
        columnsLeft = new bool[n];

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                freeSpaceInColumns[i] = n;
            }
            columnsLeft[i] = true;
        }


        layerColumns = new int[n]; // which column each layer refers to
        layerRows = new int[n]; // which row each layer is currently exploring
        currentLayer = 0; // the current layer
        layerColumns[0] = n/2; // maximizes the number of blockages in the iniitial selection
        
        columnsLeft[layerColumns[0]] = false;

        slopes.reserve(n);
    }

    Solver(Solver* s) {
        n = s->n;
        master = s->master;
        currentLayer = s->currentLayer;

        spotAvailable = new bool*[n];
        for (int i = 0; i < n; i++) {
            spotAvailable[i] = new bool[n];
            for (int j = 0; j < n; j++) {
                spotAvailable[i][j] = s->spotAvailable[i][j];
            }
        }

        invalidations = new vector<pair<int, int>>[n];
        for (int i = 0; i < n; i++) {
            invalidations[i].reserve(n*5);
        }

        for (int i = 0; i < currentLayer; i++) {
            for (int j = 0; j < s->invalidations[i].size(); j++) {
                invalidations[i].push_back(s->invalidations[i][j]);
            }
        }

        freeSpaceInColumns = new int[n];
        columnsLeft = new bool[n];
        for (int i = 0; i < n; i++) {
            freeSpaceInColumns[i] = s->freeSpaceInColumns[i];
            columnsLeft[i] = s->columnsLeft[i];
        }

        layerColumns = new int[n];
        for (int i = 0; i <= currentLayer; i++) {
            layerColumns[i] = s->layerColumns[i];
        }
        
        layerRows = new int[n];
        for (int i = 0; i < currentLayer; i++) {
            layerRows[i] = s->layerRows[i];
        }

        slopes.reserve(n);
    }

    int row() {
        return layerRows[currentLayer];
    }

    int col() {
        return layerColumns[currentLayer];
    }

    void invalidateSpot(int spotRow, int spotCol) {
        if (!spotAvailable[spotRow][spotCol]) {
            return;
        }
        spotAvailable[spotRow][spotCol] = false;
        invalidations[currentLayer].push_back(make_pair(spotRow, spotCol));
        freeSpaceInColumns[spotCol] -= 1;
    }

    void invalidate_simple() {
        int row = this->row();
        int col = this->col();
        for (int i = 0; i < n; i++) {
            // Rows and cols
            invalidateSpot(i, col);
            invalidateSpot(row, i);
        }

        // Diagonals
        for (int i = 0; row + i < n && col + i < n; i++) {
            invalidateSpot(row + i, col + i);
        }
        for (int i = 0; 0 <= row - i && col + i < n; i++) {
            invalidateSpot(row - i, col + i);
        }
        for (int i = 0; row + i < n && 0 <= col - i; i++) {
            invalidateSpot(row + i, col - i);
        }
        for (int i = 0; 0 <= row - i &&  0 <= col - i; i++) {
            invalidateSpot(row - i, col - i);
        }
        
        for (int i = 0; i < currentLayer; i++) {
            int rowStep = row - layerRows[i];
            int colStep = col - layerColumns[i];
            int gcd = __gcd(rowStep, colStep);
            rowStep /= gcd;
            colStep /= gcd;
            for (int i = 0; 0 <= (row + rowStep * i) && (row + rowStep * i) < n && 0 <= (col + colStep * i) && (col + colStep * i) < n; i++) {
                invalidateSpot(row + rowStep * i, col + colStep * i);
            }
            for (int i = 0; 0 <= (row - rowStep * i) && (row - rowStep * i) < n && 0 <= (col - colStep * i) && (col - colStep * i) < n; i++) {
                invalidateSpot(row - rowStep * i, col - colStep * i);
            }
        }
    }

    void invalidate_columns() {
        int row = this->row();
        int col = this->col();
        
        slopes.clear();

        for (int i = 0; i < currentLayer; i++) {
            int gcd = __gcd(row - layerRows[i], col - layerColumns[i]);
            slopes.push_back(make_pair((row - layerRows[i])/gcd, (col - layerColumns[i])/gcd));
        }

        for (int aCol = 0; aCol < n; aCol++) {
            if (!columnsLeft[aCol]) {
                continue;
            }
            // row
            // column (doesn't matter)
            // diagonal
            // lines * calculate slopes first, then check for each col
            invalidateSpot(row, aCol); // row

            int diff = abs(col - aCol);
            if (row + diff < n) {
                invalidateSpot(row + diff, aCol);
            }
            if (row - diff >= 0) {
                invalidateSpot(row - diff, aCol);
            }
            for (int i = 0; i < slopes.size(); i++) {
                int numSteps = (aCol - col)/slopes[i].second;
                if (numSteps*slopes[i].second == (aCol - col)) {
                    int collisonRow = row+ slopes[i].first * numSteps;
                    if (0 <= collisonRow && collisonRow < n) {
                        invalidateSpot(collisonRow, aCol);
                    }
                }
            }
        }
    }

    void invalidate() {
        invalidate_columns();
    }

    void revalidate() {
        for (int i = 0; i < invalidations[currentLayer].size(); i++) {
            int spotRow = invalidations[currentLayer][i].first;
            int spotCol = invalidations[currentLayer][i].second;
            spotAvailable[spotRow][spotCol] = true;
            freeSpaceInColumns[spotCol] += 1;
        }
        invalidations[currentLayer].clear();
    }

    bool solve(bool threadChildren) {

        if (solutionFound) {
            return false;
        }

        if (currentLayer > master->maxDepth) {
            master->maxDepth = currentLayer;
            cout << "Acheived new maxDepth of " << master->maxDepth << endl;
            cout << getSolutionString() << endl;
        }

        int col = this->col();
        int row = n/2; // Start in the middle then oscillate out
        int direction = 1;
        for (int delta = 0; delta < n; delta++) {
            row += direction * delta;
            direction *= -1;

            if (!spotAvailable[row][col]) {
                continue;
            }
            // invalidate everything
            layerRows[currentLayer] = row;

            if (currentLayer == n-1) {
                return true;
            }

            invalidate();

            int minCol = 0;
            while (!columnsLeft[minCol])
                minCol++;
            int minSize = freeSpaceInColumns[0];
            int minDist = abs(n/2 - minCol);
            for (int aCol = 1; aCol < n; aCol++) {
                if (columnsLeft[aCol] && (freeSpaceInColumns[aCol] < minSize || (freeSpaceInColumns[aCol] == minSize && abs(n/2 - aCol) < minDist))) {
                    minCol = aCol;
                    minSize = freeSpaceInColumns[aCol];
                    minDist = abs(n/2 - aCol);
                }
            }
            
            if (minSize == 0) {
                // path leads to dead end, try next option
                revalidate();
            } else {
                // expand next column
                layerColumns[++currentLayer] = minCol;
                columnsLeft[minCol] = false;

                if (threadChildren) {
                    master->enqueue(this);
                } else {
                    if (solve(false)) {
                        return true;
                    }
                }
                columnsLeft[minCol] = true;
                currentLayer--;
                revalidate();
            }
        }

        return false;
    }

    bool solve() {
        return solve(false);
    }

    void solveAndNotify() {
        if (solve()) {
            master->markSolved(this);
        }
        master->startThread();
    }

    vector<int> getSolution() {
        vector<int> sol(n);
        for (int i = 0; i < n; i++) {
            sol[i] = n*2-1;
        }
        for (int i = 0; i < n; i++) {
            sol[layerColumns[i]] = layerRows[i];
        }
        return sol;
    }

    string getSolutionString() {
        vector<int> solution = getSolution();
        string ss = to_string(solution[0] + 1);
        for (int i = 1; i < solution.size(); i++) {
            ss += " " + to_string(solution[i] + 1);
        }
        return ss;
    }
};


SolutionMaster::SolutionMaster(int n, int t):n(n),nThreads(t) {
    solvers.reserve(n*n / 2);
    active.reserve(n*n / 2);
}

void SolutionMaster::enqueue(Solver* old) {
    Solver s(old);
    solvers.push_back(s);
}

void SolutionMaster::markSolved(Solver* s) {
    solution = s->getSolution();
    for (int i = 0; i < solvers.size(); i++) {
        solvers[i].solutionFound = true;
    }
}

void SolutionMaster::startThread() {
    m.lock();
    if (nextSolver < solvers.size()) {
        thread t(&Solver::solveAndNotify, &solvers[nextSolver]);
        active.push_back(move(t));
        nextSolver++;
    }
    m.unlock();
}

vector<int> SolutionMaster::getSolution() {
    // Start threads here
    Solver s(n, this);
    s.solve(true); // Enqueues all solvers (n^2/2)

    for (int i = 0; i < nThreads; i++) {
        startThread();
    }
    
    for (int i = 0; i < active.size(); i++) {
        active[i].join();
    }

    return solution;
}

string solutionStr(int n, vector<int> solution) {
    string ss = to_string(n) + "\n";
    if (solution.empty()) { // TODO Crashes because it solution is never empty, only incomplete
        return ss + "NO SOLUTION FOUND";
    }

    ss += to_string(solution[0] + 1);
    for (int i = 1; i < solution.size(); i++) {
        ss += " " + to_string(solution[i] + 1);
    }
    return ss;
}

int main() {
    int nThreads = 8;
    ofstream file;

    // vector<int> ns = {353, 503, 505, 513, 999, 1001};
    vector<int> ns = {999, 1001};

    for (int i = 0; i < ns.size(); i++) {
        int n = ns[i];
    // for (int n = 1; n < 1000; n += 2) {
        cout << "Solving for " << n << endl;
        SolutionMaster m(n, nThreads);

        vector<int> solution = m.getSolution();

        string ss;
        ss = solutionStr(n, solution);
        file.open("output/" + to_string(n) + ".txt");
        file << ss;
        file.close();
        cout << ss << endl;
    }
    
    return 0;
}