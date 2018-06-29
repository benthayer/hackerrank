#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <set>
#include <deque>
#include <queue>
#include <algorithm>

using namespace std;


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

class GeneTreeNode {
    public:
    int min;
    long long health = 0; // undefined unless leaf
    GeneTreeNode* left;
    GeneTreeNode* right;
    GeneTreeNode() {
        left = nullptr;
        right = nullptr;
    }
    GeneTreeNode(int p, long long h):min(p),health(h) {
        left = nullptr;
        right = nullptr;
    }
    GeneTreeNode(GeneTreeNode* l, GeneTreeNode* r):left(l),right(r) {
        min = left->min;
    }

    long long getHealth(int position) {
        if (left == nullptr) {
            if (position < min) {
                return 0;
            } else {
                return health;
            }
        } else {
            if (position < right->min) {
                return left->getHealth(position);
            } else {
                return right->getHealth(position);
            }
        }
    }
};

GeneTreeNode* convert(vector<Gene*>& genes) {
    if (genes.empty()) {
        return new GeneTreeNode(0, 0);
    }
    long long sum = 0;
    vector<GeneTreeNode*>* nodes = new vector<GeneTreeNode*>;
    for (int i = 0; i < genes.size(); i++) {
        sum += genes[i]->health;
        nodes->push_back(new GeneTreeNode(genes[i]->position, sum));
    }

    vector<GeneTreeNode*>* nextNodes = new vector<GeneTreeNode*>;
    while (nodes->size() != 1) {
        for (int i = 0; i+1 < nodes->size(); i+=2) {
            GeneTreeNode* node1 = nodes->at(i);
            GeneTreeNode* node2 = nodes->at(i+1);

            nextNodes->push_back(new GeneTreeNode(node1, node2));
        }
        if(!nodes->empty()) {
            GeneTreeNode* node1 = nextNodes->back();
            nextNodes->pop_back();
            GeneTreeNode* node2 = nodes->back();

            nextNodes->push_back(new GeneTreeNode(node1, node2));
        }
        vector<GeneTreeNode*>* temp = nextNodes;
        nextNodes = nodes;
        nodes = temp;
        
        nextNodes->clear();
    }
    return nodes->front();
}


class TreeNode {
    private:
    TreeNode* children[26]; // Null pointer if no genes
    public:
    
    vector<Gene*> genes;
    GeneTreeNode* geneTree;

    TreeNode() {
        for (int i = 0; i < 26; i++) {
            children[i] = nullptr;
        }
        geneTree = nullptr;
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
        return geneTree->getHealth(dna.last) - geneTree->getHealth(dna.first - 1);
    }
};

long long getHealth(TreeNode* root, DNA& dna) {

    long long health = 0;

    TreeNode* node;
    for (int i = 0; i < dna.d.length(); i++) {
        node = root;
        for (int j = i; j < dna.d.length(); j++) {
            if (node->hasChild(dna.d[j])) {
                node = node->getChild(dna.d[j]);
                health += node->getHealth(dna);
            } else {
                break;
            }
        }
    }
    return health;
}

string process(vector<DNA>& dna, vector<string>& genes, vector<int>& healths) {
    cout << "Creating Tree" << endl;
    TreeNode* root = new TreeNode;

    set<TreeNode*> nodeSet;

    for (int i = 0; i < genes.size(); i++) {
        TreeNode* node = root;
        for (int j = 0; j < genes[i].length(); j++) {
            node = node->getChild(genes[i][j]);
            nodeSet.insert(node);
        }
        node->genes.push_back(new Gene(i, healths[i]));
    }

    cout << "Tree created" << endl;

    cout << "Creating Gene Trees" << endl;
    vector<TreeNode*> nodes(nodeSet.begin(), nodeSet.end());
    cout << "Conversion to vector complete" << endl;

    for (vector<TreeNode*>::iterator it = nodes.begin(); it != nodes.end(); it++) {
        TreeNode* node = *it;
        node->geneTree = convert(node->genes);
    }

    cout << "Gene Trees created" << endl;

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