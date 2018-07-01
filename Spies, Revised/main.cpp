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
    bool** boardAvailability;
    vector<pair<int, int>>* invalidations;
    set<int>* freeSpacesInColumns;
    int* layerColumns;
    int* layerRows;
    set<int> columnsLeft;
    int currentLayer;
    int n;
    double duration;
    clock_t start;
    Solver(int n):n(n),duration() {

        boardAvailability = new bool*[n];
        for (int i = 0; i < n; i++) {
            boardAvailability[i] = new bool[n];
            for (int j = 0; j < n; j++) {
                boardAvailability[i][j] = true;
            }
        }
        

        invalidations = new vector<pair<int, int>>[n]; // spots layer has invalidated
        freeSpacesInColumns = new set<int>[n]; // available spots in the column

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                freeSpacesInColumns[i].insert(j);
            }
            columnsLeft.insert(i);
        }


        layerColumns = new int[n]; // which column each layer refers to
        layerRows = new int[n]; // which row each layer is currently exploring
        currentLayer = 0; // the current layer
        layerColumns[0] = n/2; // maximizes the number of blockages in the iniitial selection
        
        columnsLeft.erase(layerColumns[0]);
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
        if (!boardAvailability[spotRow][spotCol]) {
            return;
        }
        boardAvailability[spotRow][spotCol] = false;
        invalidations[currentLayer].push_back(make_pair(spotRow, spotCol));
        freeSpacesInColumns[spotCol].erase(spotRow);
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

    void invalidate() {
        vector<pair<int, int>> slopes;
        for (int i = 0; i < currentLayer; i++) {
            int gcd = __gcd(row() - layerRows[i], col() - layerColumns[i]);
            slopes.push_back(make_pair((row() - layerRows[i])/gcd, (col() - layerColumns[i])/gcd));
        }
        for (auto aCol : columnsLeft) {
            // row
            // column (doesn't matter)
            // diagonal
            // lines * calculate slopes first, then check for each col
            invalidateSpot(row(), aCol); // row

            int diff = abs(col() - aCol);
            if (row() + diff < n) {
                invalidateSpot(row() + diff, aCol);
            }
            if (row() - diff >= 0) {
                invalidateSpot(row() - diff, aCol);
            }
            for (int i = 0; i < slopes.size(); i++) {
                int numSteps = (aCol - col())/slopes[i].second;
                if (numSteps*slopes[i].second == (aCol - col())) {
                    int collisonRow = row() + slopes[i].first * numSteps;
                    if (0 <= collisonRow && collisonRow < n) {
                        invalidateSpot(collisonRow, aCol);
                    }
                }
            }
        }
    }

    void revalidate() {
        for (int i = 0; i < invalidations[currentLayer].size(); i++) {
            int spotRow = invalidations[currentLayer][i].first;
            int spotCol = invalidations[currentLayer][i].second;
            boardAvailability[spotRow][spotCol] = true;
            freeSpacesInColumns[spotCol].insert(spotRow);
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


        vector<int> freeSpaces(freeSpacesInColumns[col()].begin(), freeSpacesInColumns[col()].end());
        sort(freeSpaces.begin(), freeSpaces.end(), [=] (int a, int b) {
            return abs(a-n/2) < abs(b-n/2);
        });

        for (vector<int>::iterator spy = freeSpaces.begin(); spy != freeSpaces.end(); spy++) {
            // invalidate everything
            layerRows[currentLayer] = *spy;

            if (currentLayer == n-1) {
                return true;
            }

            invalidate_simple();
            
            // check for next column
            set<int>::iterator col = columnsLeft.begin();
            int minCol = *col; // could keep track of vector of all columns with the smallest then choose the centermost
            int minSize = freeSpacesInColumns[*col].size();
            int minCenter = abs(n/2 - minCol);
            col++;
            for ( ; col != columnsLeft.end(); col++) {
                // cout << "Column " << *col << ": " << freeSpacesInColumns[*col].size() << endl;
                if (freeSpacesInColumns[*col].size() < minSize || (freeSpacesInColumns[*col].size() == minSize && abs(n/2 - *col) < minCenter)) {
                    minCol = *col;
                    minSize = freeSpacesInColumns[*col].size();
                }
            }
            
            if (minSize == 0) {
                // path leads to dead end, try next option
                revalidate();
            } else {
                // expand next column
                layerColumns[++currentLayer] = minCol;
                columnsLeft.erase(minCol);
                
                if (solve()) {
                    return true;
                }
                columnsLeft.insert(minCol);
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
    for (int i = 0; i < solution.size(); i++) {
        ss += " " + to_string(solution[i] + 1);
    }
    return ss;
}

int main() {
    ofstream file;
    vector<int> ns = {113, 119, 129, 153, 155, 157, 159, 203,213};
    for (int i = 0; i < ns.size(); i++) {
        int n = ns[i];
        cout << "Solving for " << n << endl;
        Solver solver(n);
        clock_t start = clock();
        bool solved = solver.solve();
        double time = clock() - start;
        // cout << solver.duration << "s of " << time << "s spent calculating columns (" << 100 * solver.duration / time << "%)" << endl;
        string ss;
        ss = solutionStr(solver, solved);
        file.open("output\\" + to_string(n) + ".txt");
        file << ss;
        file.close();
        cout << ss << endl;
        if (n==11 && !solved) {
            cout << "11 not solved" << endl;
        }
    }
    
    return 0;
}