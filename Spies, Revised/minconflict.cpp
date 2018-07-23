#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>
#include <stdlib.h>
#include <time.h>

using namespace std;

class Solver {
    public:
    int n;

    vector<vector<int>> numConflicts;
    vector<vector<pair<int, int>>> simpleInvalidations;
    vector<vector<vector<pair<int, int>>>> colinearInvalidations;

    vector<int> rows;

    vector<vector<int>> placementOptions;
    vector<int> changableColumns;

    bool initialized = false;

    Solver(int n):n(n) {
        srand(time(NULL));

        numConflicts.reserve(n);
        for (int i = 0; i < n; i++) {
            numConflicts.push_back(vector<int>());
            numConflicts[i].reserve(n);
            for (int j = 0; j < n; j++) {
                numConflicts[i].push_back(0);
            }
        }

        simpleInvalidations.reserve(n);
        for (int i = 0; i < n; i++) {
            simpleInvalidations.push_back(vector<pair<int, int>>());
            simpleInvalidations[i].reserve(3*n);
        }

        colinearInvalidations.reserve(n);
        for (int i = 0; i < n; i++) {
            colinearInvalidations.push_back(vector<vector<pair<int, int>>>());
            colinearInvalidations[i].reserve(n);
            for (int j = 0; j < n; j++) {
                colinearInvalidations[i].push_back(vector<pair<int, int>>());
            }
        }

        placementOptions.reserve(n);
        for (int i = 0; i < n; i++) {
            placementOptions.push_back(vector<int>());
        }
        
        for (int i = 0; i < n; i++) {
            rows.push_back(n * 2 - 1);
        }
        
        for (int col = 0; col < n; col++) {
            rows[col] = rand() % n;

            revalidate(col);
            invalidate(col);
        }
        initialized = true;

    }

    Solver(vector<int> sol):n(sol.size()) {

        numConflicts.reserve(n);
        for (int i = 0; i < n; i++) {
            numConflicts.push_back(vector<int>());
            numConflicts[i].reserve(n);
            for (int j = 0; j < n; j++) {
                numConflicts[i].push_back(0);
            }
        }

        simpleInvalidations.reserve(n);
        for (int i = 0; i < n; i++) {
            simpleInvalidations.push_back(vector<pair<int, int>>());
            simpleInvalidations[i].reserve(3*n);
        }

        colinearInvalidations.reserve(n);
        for (int i = 0; i < n; i++) {
            colinearInvalidations.push_back(vector<vector<pair<int, int>>>());
            colinearInvalidations[i].reserve(n);
            for (int j = 0; j < n; j++) {
                colinearInvalidations[i].push_back(vector<pair<int, int>>());
            }
        }

        placementOptions.reserve(n);
        for (int i = 0; i < n; i++) {
            placementOptions.push_back(vector<int>());
        }
        
        rows.reserve(n);
        for (int i = 0; i < n; i++) {
            rows.push_back(n * 2 - 1);
        }

        for (int col = 0; col < n; col++) {
            if (sol[col] == 2*n - 1) {
                continue;
            }

            revalidate(col);
            rows[col] = sol[col];
            invalidate(col);
        }

        for (int col = 0; col < n; col++) {
            if (sol[col] != 2*n - 1) {
                continue;
            }
            calculatePlacementOptions(col);
            rows[col] = getPlacement(col);
            revalidate(col);
            invalidate(col);
        }
        initialized = true;
    }

    void invalidateSpot(int row, int col, int originalColumn) {
        numConflicts[row][col] += 1;
        simpleInvalidations[originalColumn].push_back(make_pair(row, col));
    }

    void invalidateSimple(int col) {
        int row = rows[col];

        // Row
        for (int i = 0; i < col; i++) {
            invalidateSpot(row, i, col);
        }
        for (int i = col + 1; i < n; i++) {
            invalidateSpot(row, i, col);
        }
        
        // Diagonals
        for (int i = 1; row + i < n && col + i < n; i++) {
            invalidateSpot(row + i, col + i, col);
        }
        for (int i = 1; 0 <= row - i && col + i < n; i++) {
            invalidateSpot(row - i, col + i, col);
        }
        for (int i = 1; row + i < n && 0 <= col - i; i++) {
            invalidateSpot(row + i, col - i, col);
        }
        for (int i = 1; 0 <= row - i &&  0 <= col - i; i++) {
            invalidateSpot(row - i, col - i, col);
        }

    }

    void invalidateColinearPair(int col1, int col2) {
        int row1 = rows[col1];
        int row2 = rows[col2];
        int rowStep = row2 - row1;
        int colStep = col2 - col1; // Always positive
        int gcd = abs(__gcd(rowStep, colStep));
        rowStep /= gcd;
        colStep /= gcd;

        if (row1 == n*2 - 1) {
            if (row2 != n*2 - 1) {
                numConflicts[row2][col2] += 1;
                colinearInvalidations[col1][col2].push_back(make_pair(row2, col2));
            }
            return;
        }

        int row;
        int col;
        if (rowStep > 0) {
            row = row1;
            col = col1;
            while (col < n && row < n) {
                numConflicts[row][col] += 1;
                colinearInvalidations[col1][col2].push_back(make_pair(row, col));
                row += rowStep;
                col += colStep;
            }
            row = row1 - rowStep;
            col = col1 - colStep;
            while (0 <= col && 0 <= row) {
                numConflicts[row][col] += 1;
                colinearInvalidations[col1][col2].push_back(make_pair(row, col));
                row -= rowStep;
                col -= colStep;
            }
        } else {
            row = row1;
            col = col1;
            while (col < n && 0 <= row) {
                numConflicts[row][col] += 1;
                colinearInvalidations[col1][col2].push_back(make_pair(row, col));
                row += rowStep;
                col += colStep;
            }
            row = row1 - rowStep;
            col = col1 - colStep;
            while (0 <= col && row < n) {
                numConflicts[row][col] += 1;
                colinearInvalidations[col1][col2].push_back(make_pair(row, col));
                row -= rowStep;
                col -= colStep;
            }
        }
    }

    void invalidateColinear(int col) {
        int row = rows[col];
        for (int i = 0; i < col; i++) {
            invalidateColinearPair(i, col);
        }

        for (int i = col+1; i < n; i++) {
            invalidateColinearPair(col, i);
        }

        numConflicts[rows[col]][col] -= n-1;
    }

    void invalidate(int col) {
        invalidateSimple(col);
        invalidateColinear(col);
    }

    void revalidateSimple(int col) {
        for (int i = 0; i < simpleInvalidations[col].size(); i++) {
            numConflicts[simpleInvalidations[col][i].first][simpleInvalidations[col][i].second] -= 1;
        }
        
        simpleInvalidations[col].clear();
        
    }

    void revalidateColinear(int col) {
        for (int i = 0; i < col; i++) {
            for (auto spot : colinearInvalidations[i][col]) {
                numConflicts[spot.first][spot.second] -= 1;
            }
            colinearInvalidations[i][col].clear();
        }

        for (int i = col+1; i < n; i++) {
            for (auto spot : colinearInvalidations[col][i]) {
                numConflicts[spot.first][spot.second] -= 1;
            }
            colinearInvalidations[col][i].clear();
        }

        if (initialized)
            numConflicts[rows[col]][col] += n-1;
    }

    void revalidate(int col) {
        revalidateSimple(col);
        revalidateColinear(col);
    }

    void calculatePlacementOptions(int col) {
        placementOptions[col].clear();
        placementOptions[col].push_back(0);
        int minConflicts = numConflicts[0][col];
        for (int row = 1; row < n; row++) {
            int conflicts = numConflicts[row][col];
            if (conflicts < minConflicts) {
                placementOptions[col].clear();
                minConflicts = conflicts;
                placementOptions[col].push_back(row);
            } else if (conflicts == minConflicts) {
                placementOptions[col].push_back(row);
            }
        }
    }

    void calculatePlacementOptions() {
        changableColumns.clear();
        for (int col = 0; col < n; col++) {
            calculatePlacementOptions(col);

            if (placementOptions[col].size() > 1 || placementOptions[col][0] != rows[col]) {
                changableColumns.push_back(col);
            }
        }
    }

    int getChangableCol() {
        return changableColumns[rand() % changableColumns.size()];
    }

    int getPlacement(int col) {
        return placementOptions[col][rand() % placementOptions[col].size()];
    }

    bool solved() {
        for (int col = 0; col < n; col++) {
            if (numConflicts[rows[col]][col] != 0) {
                return false;
            }
        }
        return true;
    }

    bool solve(int maxIter) {
        int i = 0;
        while (!solved()) {
            if (i == maxIter) {
                cout << "Max iterations" << endl;
                return false;
            } else if (i % 10 == 0) {
                int c = 0;
                vector<int> counts(10);
                int max = 0;
                for (int col = 0; col < n; col++) {
                    int l = numConflicts[rows[col]][col];
                    c += l;
                    if (l < 9) {
                        counts[l] += 1;
                    } else {
                        counts[9] += 1;
                        if (l > max) {
                            max = l;
                        }
                    }
                }
                cout << c << " conflicts (";
                for (int i = 0; i < counts.size()-1; i++) {
                    cout << i << ":" << counts[i] << ", ";
                }
                cout << "9+:" << counts[9] << ", max=" << max << ")" << endl;
            }

            calculatePlacementOptions();

            if (changableColumns.size() == 0) {
                cout << "No valid columns to change at iteration " << i << endl;
                return false;
            }

            int col = getChangableCol();

            revalidate(col);

            int row = getPlacement(col);

            rows[col] = row;
            invalidate(col);
            i++;
        }
        return true;
    }

    bool solve() {
        return solve(-1);
    }

    vector<int> getSolution() {
        return rows;
    }
};

string solutionStr(Solver& s) {
    string ss = to_string(s.n) + "\n";
    if (!s.solved()) {
        return ss + "NO SOLUTION FOUND";
    }

    vector<int> solution = s.getSolution();
    ss += to_string(solution[0] + 1);
    for (int i = 1; i < solution.size(); i++) {
        ss += " " + to_string(solution[i] + 1);
    }
    return ss;
}

void solve(Solver& s) {
    ofstream file;
    cout << "Solving for " << s.n << endl;
    s.solve();

    string ss;
    ss = solutionStr(s);
    file.open("output_minconflict/" + to_string(s.n) + ".txt");
    file << ss;
    file.close();
    cout << ss << endl;
}

void solve(int n) {
    Solver s(n);
    solve(s);
}

void solve(vector<int> sol) {
    Solver s(sol);
    solve(s);
}

void solveABunch(vector<int> sol) {
    // uses solution from the previous n to find a solution faster. 
    // calling solver for n-2 then using solution as a starting place for n is not faster than just calling the solver for n
    for (int i = sol.size(); i <= 10001; i+=2) {
        Solver s(sol);
        solve(s);
        sol = s.getSolution();
        sol.push_back(i*2+3); // +3 instead of -1 because we're adding for the next set
        sol.push_back(i*2+3);
    }
}

int main() {
    solve(2001);
    return 0;
}