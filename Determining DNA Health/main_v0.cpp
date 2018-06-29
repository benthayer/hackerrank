#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <set>
#include <algorithm>

using namespace std;

vector<string> split_string(string);


class DNA {
    public:
    int first;
    int last;
    string d;
    DNA(int f, int l, string dna):first(f),last(l),d(dna) {}
};

class Gene {
    public:
    vector<int> positions;
    vector<int> healths;
    Gene(int p, int h) {
        addPosition(p, h);
    }
    void addPosition(int p, int h) {
        positions.push_back(p);
        healths.push_back(h);
    }

    int getHealth(int first, int last) {
        int health = 0;
        for (int i = 0; i < positions.size(); i++) {
            if (first <= positions[i] && positions[i] <= last) {
                health += healths[i];
            }
        }
        return health;
    }
};

long getHealth(DNA& dna, map<string,Gene>& db, vector<int>& geneLengths) {
    long health = 0;
    
    for (int i = 0; i < dna.d.length(); i++) {
        for (int j = 0; j < geneLengths.size() && i + geneLengths[j] - 1 < dna.d.length(); j++) {
            auto it = db.find(dna.d.substr(i, geneLengths[j]));
            if (it != db.end()) {
                health += it->second.getHealth(dna.first, dna.last);
            }
        }
    }
    // cout << health << endl;
    return health;
}

string process(vector<DNA>& dna, vector<string>& genes, vector<int>& healths) {

    map<string,Gene> db;
    set<int> geneLengthsSet;
    for (int i = 0; i < genes.size(); i++) {
        geneLengthsSet.insert(genes[i].length());
        auto it = db.find(genes[i]);
        if (it == db.end()) {
            db.insert(make_pair(genes[i], Gene(i, healths[i])));
        } else {
            it->second.addPosition(i, healths[i]);
        }
    }

    vector<int> geneLengths(geneLengthsSet.begin(), geneLengthsSet.end());
    sort(geneLengths.begin(), geneLengths.end());


    long minHealth, maxHealth;
    minHealth = maxHealth = getHealth(dna[0], db, geneLengths);
    
    for (int i = 1; i < dna.size(); i++) {
        if (i % 1000 == 0) {
            cout << i << endl;
        }
        long h = getHealth(dna[i], db, geneLengths);
        if (h < minHealth) {
            minHealth = h;
        } else if (h > maxHealth) {
            maxHealth = h;
        }
    }
    return to_string(minHealth) + " " + to_string(maxHealth);
} 


int testCase(int testcase)
{
    // n
    // genes
    // health
    // test cases
    // first last d


    ifstream file;
    file.open("testcase_" + to_string(testcase) + ".txt");

    int n;
    file >> n;

    vector<string> genes;

    for (int i = 0; i < n; i++) {
        string gene;
        file >> gene;
        genes.push_back(gene);
    }

    vector<int> healths;

    for (int i = 0; i < n; i++) {
        int health;
        file >> health;

        healths.push_back(health);
    }

    int s;
    file >> s;
    
    vector<DNA> dna;
    for (int s_itr = 0; s_itr < s; s_itr++) {
        int first, last;
        string d;
        
        file >> first >> last >> d;
        
        dna.push_back(DNA(first, last, d));
    }
    
    string result = process(dna, genes, healths);

    long min, max;
    file.close();
    file.open("testcase_" + to_string(testcase) + "_output.txt");

    file >> min >> max;
    if (result == to_string(min) + " " + to_string(max)) {
        cout << "Correct answer" << endl;
    } else {
        cout << "Incorrect answer" << endl;
    }
    

    return 0;
}

int main() {
    testCase(0);
    testCase(13);
    testCase(26);
    testCase(31);

    return 0;
}