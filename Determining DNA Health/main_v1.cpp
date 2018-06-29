#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <set>
#include <algorithm>
#include <chrono>

using namespace std;
using namespace std::chrono;


class DNA {
    public:
    int first;
    int last;
    string d;
    DNA(int f, int l, string dna):first(f),last(l),d(dna) {}
};

class Gene {
    public:
    int position;
    int health;
    Gene(int p, int h):position(p),health(h) {}

    long long getHealth(int first, int last) {
        if (first <= position && position <= last) {
            return health;
        }
        return 0;
    }

    long long getHealth(DNA& dna) {
        return getHealth(dna.first, dna.last);
    }
};


class TreeNode {
    private:
    TreeNode* children[26]; // Null pointer if no genes
    public:
    
    vector<Gene*> genes;

    TreeNode() {
        for (int i = 0; i < 26; i++) {
            children[i] = nullptr;
        }
    }

    bool hasChild(char c) {
        return children[c-'a'] != nullptr;
    }

    TreeNode* getChild(char c) {
        if (!hasChild(c)) {
            children[c-'a'] = new TreeNode;
        }
        return children[c-'a'];
        
    }

    long long getHealth(DNA& dna) {
        long long health = 0;
        for (int i = 0; i < genes.size(); i++) {
            health += genes[i]->getHealth(dna);
        }
        return health;
    }
};

long long getHealth(TreeNode* root, DNA& dna) {

    map<string,long long> cache;

    long long health = 0;

    TreeNode* node;
    for (int i = 0; i < dna.d.length(); i++) {
        node = root;
        for (int j = i; j < dna.d.length(); j++) {
            if (node->hasChild(dna.d[j])) {
                node = node->getChild(dna.d[j]);
                long long h;
                string s = dna.d.substr(i, j-i);
                auto it = cache.find(s);
                if (it != cache.end()) {
                    h = it->second;
                } else {
                    h = node->getHealth(dna);
                    cache.insert(make_pair(s, h));
                }
                health += h;
            } else {
                break;
            }
        }
    }
    return health;
}

void insertGene(TreeNode* root, Gene* gene, string& s) {
    TreeNode* node = root;
    for (int i = 0; i < s.length(); i++) {
        node = node->getChild(s[i]);
    }
    node->genes.push_back(gene);
}

string process(vector<DNA>& dna, vector<string>& genes, vector<int>& healths) {
    cout << "Creating Tree" << endl;
    TreeNode* root = new TreeNode;

    for (int i = 0; i < genes.size(); i++) {
        insertGene(root, new Gene(i, healths[i]), genes[i]);
    }

    cout << "Tree created" << endl;


    long long minHealth, maxHealth;
    minHealth = maxHealth = getHealth(root, dna[0]);
    
    for (int i = 1; i < dna.size(); i++) {
        long long h = getHealth(root, dna[i]);
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
    cout << "Starting" << endl;
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
    cout << "Processing" << endl;
    string result = process(dna, genes, healths);
    cout << "Done Processing" << endl;

    long long min, max;
    file.close();
    file.open("testcase_" + to_string(testcase) + "_output.txt");

    file >> min >> max;
    string answer = to_string(min) + " " + to_string(max);
    
    cout << "Result: " << result << endl;
    cout << "Answer: " << answer << endl;

    if (result == answer) {
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