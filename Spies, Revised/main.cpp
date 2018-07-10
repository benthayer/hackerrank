#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <chrono>

using namespace std;
using namespace std::chrono;

class Solver {
    public:
    int n;

    bool** spotAvailable;
    vector<pair<int, int>>* invalidations;
    int* freeSpaceInColumns;
    bool* columnsLeft;

    int* layerColumns;
    int* layerRows;
    int currentLayer;

    vector<pair<int, int>> slopes;

    double duration = 0;
    clock_t start;
    Solver(int n):n(n) {

        spotAvailable = new bool*[n];
        for (int i = 0; i < n; i++) {
            spotAvailable[i] = new bool[n];
            for (int j = 0; j < n; j++) {
                spotAvailable[i][j] = true;
            }
        }
        

        invalidations = new vector<pair<int, int>>[n]; // spots layer has invalidated
        
        freeSpaceInColumns = new int[n];
        columnsLeft = new bool[n];

        for (int i = 0; i < n; i++) {
            freeSpaceInColumns[i] = n;
            columnsLeft[i] = true;
        }


        layerColumns = new int[n]; // which column each layer refers to
        layerRows = new int[n]; // which row each layer is currently exploring
        currentLayer = 0; // the current layer
        layerColumns[0] = n/2; // maximizes the number of blockages in the iniitial selection
        
        columnsLeft[layerColumns[0]] = false;
    }

    void tStart() {
        start = clock();
    }

    void tStop() {
        duration += clock() - start;
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
        invalidations[currentLayer].push_back(make_pair(spotRow, spotCol)); // 5%ish
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
                    int collisonRow = row + slopes[i].first * numSteps;
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

    bool solve() {
        // cout << "Solve called" << endl;
        /*
        Rules:
        None can share the same row/column
        None can see each other diagonally
        No 3 can be in any line
        */
    
        // naive first
        // create data structure that enables and disables available spaces as needed

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
                
                if (solve()) {
                    return true;
                }
                columnsLeft[minCol] = true;
                currentLayer--;
                revalidate();
            }
        }


        return false;
    }

    vector<int> getSolution() {
        vector<int> sol(n);
        for (int i = 0; i < n; i++) {
            sol[layerColumns[i]] = layerRows[i];
        }
        return sol;
    }
};

string solutionStr(Solver solver, bool solved) {
    string ss = to_string(solver.n) + "\n";
    if (!solved) {
        return ss + "NO SOLUTION FOUND";
    }
    vector<int> solution = solver.getSolution();
    ss += to_string(solution[0] + 1);
    for (int i = 1; i < solution.size(); i++) {
        ss += " " + to_string(solution[i] + 1);
    }
    return ss;
}

int main() {
    ofstream file;
    // vector<int> ns = {113, 119, 129, 153, 155, 157, 159, 203,213};
    // for (int i = 0; i < ns.size(); i++) {
        // int n = ns[i];
    for (int n = 1; n < 50; n+=2) {
        cout << "Solving for " << n << endl;
        Solver solver(n);

        clock_t start = clock();
        bool solved = solver.solve();
        double time = clock() - start;

        cout << solver.duration << " of " << time << " spent calculating marked areas (" << 100 * solver.duration / time << "%)" << endl;

        string ss;
        ss = solutionStr(solver, solved);
        file.open("output\\" + to_string(n) + ".txt");
        file << ss;
        file.close();
        cout << ss << endl;
    }
    
    return 0;
}