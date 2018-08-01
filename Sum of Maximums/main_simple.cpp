#include <bits/stdc++.h>

using namespace std;

class QueryNode {
    public:
    static long long* A;
    static int left;
    static int right;
    static int rightmost;
    
    int position;
    int leftDominance = 0;
    int rightDominance = 0;
    long long childSum = 0;
    QueryNode* child = nullptr; // less than, pos < ch->pos
    QueryNode* next = nullptr; // greater than, ch->pos = pos+rD+1
    
    QueryNode(int position):position(position) {}

    long long getSum() {
        if (position > rightmost) {
            rightmost = position + rightDominance;
        }
        if (position > right) {
            return 0;
        }
        
        long long sum = 0;
        if (position >= left) {
            long long leftFactor = min(leftDominance + 1, position - left + 1);
            long long rightFactor = min(rightDominance + 1, right - position + 1);
            
            sum = leftFactor * rightFactor * A[position];
        
        }
        
        if (position + rightDominance >= left) {
            if (position >= left && position + rightDominance < right) {
                sum += childSum;
            } else if (child) {
                sum += child->getSum();
            }
        }
        
        if (next) {
            sum += next->getSum();
        }
        
        return sum;
    }
};

long long* QueryNode::A = nullptr;
int QueryNode::left = -1;
int QueryNode::right = -1;
int QueryNode::rightmost = -1;


QueryNode** buildNodes(long long* A, int n) {
    QueryNode::A = A;
    QueryNode** nodes = new QueryNode*[n];
        
    vector<QueryNode*> stack;
    
    
    nodes[0] = new QueryNode(0);
    nodes[0]->leftDominance = 0;
    stack.push_back(nodes[0]);
    
    QueryNode* lastNode = nullptr;
    
    for (int i = 1; i < n; i++) {
        
        while (!stack.empty() && A[stack.back()->position] < A[i]) {
            stack.back()->rightDominance = i - stack.back()->position - 1;

            if (stack.size() >= 2) {
                long long sum = A[stack.back()->position];
                sum *= (long long) (stack.back()->leftDominance + 1);
                sum *= (long long) (stack.back()->rightDominance + 1);
                stack[stack.size()-2]->childSum += sum + stack.back()->childSum;
            }
            
            lastNode = stack.back();
            stack.pop_back();
        }
        
        nodes[i] = new QueryNode(i);
        if (stack.empty()) {
            // This is the largest thing yet
            nodes[i]->leftDominance = i;
        } else {
            nodes[i]->leftDominance = i - stack.back()->position - 1;
        }
        
        // all nodes being added are a child node or a next node
        if (lastNode) {
            lastNode->next = nodes[i];
            lastNode = nullptr;
        } else {
            stack[stack.size()-1]->child = nodes[i];
        }
        stack.push_back(nodes[i]);
    }
    
    while (!stack.empty()) {
        stack.back()->rightDominance = n - stack.back()->position - 1;

        if (stack.size() >= 2) {
            long long sum = A[stack.back()->position];
            sum *= (long long) (stack.back()->leftDominance + 1);
            sum *= (long long) (stack.back()->rightDominance + 1);
            stack[stack.size()-2]->childSum += sum;
        }
        
        stack.pop_back();
    }
    return nodes;
}


long long query(QueryNode** nodes, int left, int right) {
    QueryNode::left = left;
    QueryNode::right = right;
    QueryNode::rightmost = left-1;
    
    long long sum = 0;
    while(QueryNode::rightmost+1 <= right) {
        sum += nodes[QueryNode::rightmost+1]->getSum();
    }
    return sum;
}
    

int main()
{   
    int n, m;
    scanf("%d %d\n", &n, &m);
    
    long long* A = new long long[n];
    for (int i = 0; i < n; i++) {
        scanf("%lld", &A[i]);
    }
    
    QueryNode** nodes = buildNodes(A, n);
    
    int left, right;
    for (int i = 0; i < m; i++) {
        scanf("%d %d", &left, &right);
        printf("%lld\n", query(nodes, left-1, right-1));
    }
    return 0;
}
