#include <bits/stdc++.h>

using namespace std;

class Sequence {
    public:
    int start; // for reverse sequences, start > end
    int end;
    Sequence* superior = nullptr;
    set<int> elements;
    set<int> breakpoints;
    
    Sequence(int i) {
        start = i;
        end = i;
        elements.insert(i);
    }
    
    void add(int i) {
        end = i;
        elements.insert(i);
    }
};

class QueryNode {
    public:
    static int n;
    static long long* A;
    static QueryNode** nodes;
    
    int position;
    int leftDominance = 0;
    int rightDominance = 0;
    long long rightChildSum = 0;
    long long leftChildSum = 0;
    long long lDNumSum = 0;
    long long rDNumSum = 0;
    long long lDNumIdxSum = 0;
    long long rDNumIdxSum = 0;
    QueryNode* child = nullptr; // less than, pos < ch->pos
    QueryNode* next = nullptr; // greater than, ch->pos = pos+rD+1
    QueryNode* rChild = nullptr;
    QueryNode* rNext = nullptr;
    
    Sequence* ascendSequence = nullptr;
    Sequence* rAscendSequence = nullptr;
    
    QueryNode(int position):position(position) {}
    
    int start() {
        return position - leftDominance;
    }
    
    int end() {
        return position + rightDominance;
    }
    
    long long lDNum() {
        return ((long long) (leftDominance + 1)) * A[position];
    }
    
    long long lDNumIdx() {
        return ((long long) position) * lDNum();
    }
    
    long long rDNum() {
        return ((long long) (rightDominance + 1)) * A[position];
    }
    
    long rDNumIdx() {
        return ((long long) position) * rDNum();
    }
    
    long long singleSum(int left, int right) {
        long long leftFactor = min(leftDominance + 1, position - left + 1);
        long long rightFactor = min(rightDominance + 1, right - position + 1);

        return leftFactor * rightFactor * A[position];
    }

    long long getSum(int left, int right) {
        if (position > right) {
            return 0;
        }
        
        long long sum = 0;
        if (position >= left) {
            sum = singleSum(left, right);
        }
        
        if (end() >= left) {
            if (position >= left && end() <= right) {
                sum += rightChildSum;
            } else {
                QueryNode* nextChild = child;
                while (nextChild != nullptr) {
                    sum += nextChild->getSum(left, right);
                    nextChild = nextChild->next;
                }
            }
        }
        
        return sum;
    }

    long long getRSum(int left, int right) {
        if (position < left) {
            return 0;
        }
        
        long long sum = 0;
        if (position <= right) {
            sum = singleSum(left, right);
        }
        
        if (start() <= right) {
            if (position <= right && start() >= left) {
                sum += leftChildSum;
            } else {
                QueryNode* nextChild = rChild;
                while (nextChild != nullptr) {
                    sum += nextChild->getRSum(left, right);
                    nextChild = nextChild->rNext;
                }
            }
        }
        
        return sum;
    }
};

long long* QueryNode::A = nullptr;
int QueryNode::n = 0;
QueryNode** QueryNode::nodes = nullptr;

class MaxFinderNode {
    public:
    static MaxFinderNode* root;
    int maxPosition;
    int rangeStart;
    int rangeEnd;
    MaxFinderNode* left;
    MaxFinderNode* right;
    
    MaxFinderNode(int pos):maxPosition(pos),rangeStart(pos),rangeEnd(pos) {}
    
    MaxFinderNode(MaxFinderNode* left, MaxFinderNode* right) {
        this->left = left;
        this->right = right;
        rangeStart = left->rangeStart;
        rangeEnd = right->rangeEnd;
        
        long long*& A = QueryNode::A;
        if (A[left->maxPosition] >= A[right->maxPosition]) {
            maxPosition = left->maxPosition;
        } else {
            maxPosition = right->maxPosition;
        }
    }
    
    
    long long value() {
        return QueryNode::A[maxPosition];
    }
};

MaxFinderNode* MaxFinderNode::root = nullptr;

int getMax(int left, int right) {
    long long*& A = QueryNode::A;
    auto comp = [](MaxFinderNode* a, MaxFinderNode* b) {
        if (a->value() == b->value()) {
            // define that smaller position implies greater value
            return a->maxPosition > b->maxPosition;
        }
        return a->value() < b->value();
    };
    priority_queue<MaxFinderNode*, vector<MaxFinderNode*>, decltype(comp)> toExpand(comp);
    toExpand.push(MaxFinderNode::root);
    
    while (!toExpand.empty()) {
        MaxFinderNode* expand = toExpand.top();
        toExpand.pop();
        int maxPos = expand->maxPosition;
        
        if ((left <= maxPos) && (maxPos <= right)) {
            return maxPos;
        }
        
        // nodes either have left and right or neither
        if (expand->left) {
            if (right <= expand->left->rangeEnd) {
                toExpand.push(expand->left);
            } else if (left >= expand->right->rangeStart) {
                toExpand.push(expand->right);
            } else {
                toExpand.push(expand->left);
                toExpand.push(expand->right);
            }
        }
    }
    return -1;
}


void buildMaxFinder() {
    int& n = QueryNode::n;
    long long*& A = QueryNode::A;
    
    deque<MaxFinderNode*> nodes;
    
    for (int i = 0; i < n; i++) {
        nodes.push_back(new MaxFinderNode(i));
    }
    
    while (nodes.size() != 1) {
        if (nodes[0]->maxPosition > nodes[1]->maxPosition) {
            nodes.push_back(nodes.front());
            nodes.pop_front();
        } else {
            nodes.push_back(new MaxFinderNode(nodes[0], nodes[1]));
            nodes.pop_front();
            nodes.pop_front();
        }
    }
    MaxFinderNode::root = nodes.front();
}


void buildNodes() {
    int& n = QueryNode::n;
    long long*& A = QueryNode::A;
    QueryNode**& nodes = QueryNode::nodes;
    nodes = new QueryNode*[n];
        
    vector<QueryNode*> stack;
    
    
    nodes[0] = new QueryNode(0);
    nodes[0]->leftDominance = 0;
    nodes[0]->ascendSequence = new Sequence(0);
    nodes[0]->rDNumSum = 0;
    nodes[0]->rDNumIdxSum = 0;
    stack.push_back(nodes[0]);
    
    QueryNode* lastNode = nullptr;
    
    for (int i = 1; i < n; i++) {
        vector<QueryNode*> popped;
        while (!stack.empty() && A[stack.back()->position] < A[i]) {
            stack.back()->rightDominance = i - stack.back()->position - 1;

            if (stack.size() >= 2) {
                long long sum = A[stack.back()->position];
                sum *= (long long) (stack.back()->leftDominance + 1);
                sum *= (long long) (stack.back()->rightDominance + 1);
                stack[stack.size()-2]->rightChildSum += sum + stack.back()->rightChildSum;
            }
            
            lastNode = stack.back();
            popped.push_back(stack.back());
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
            nodes[i]->ascendSequence = lastNode->ascendSequence;
            nodes[i]->ascendSequence->add(i);
            
            if (lastNode->position + 1 != i) {
                nodes[i]->ascendSequence->breakpoints.insert(lastNode->position);
            }
            
            popped.pop_back(); // Removes lastNode
            while (!popped.empty()) {
                popped.back()->ascendSequence->superior = nodes[i]->ascendSequence;
                popped.pop_back();
            }
            
            nodes[i]->rDNumSum = lastNode->rDNumSum + lastNode->rDNum();
            nodes[i]->rDNumIdxSum = lastNode->rDNumIdxSum + lastNode->rDNumIdx();
            lastNode = nullptr;
        } else {
            stack[stack.size()-1]->child = nodes[i];
            nodes[i]->ascendSequence = new Sequence(i);
            
            nodes[i]->rDNumSum = 0;
            nodes[i]->rDNumIdxSum = 0;
        }
        stack.push_back(nodes[i]);
    }
    
    while (!stack.empty()) {
        stack.back()->rightDominance = n - stack.back()->position - 1;

        if (stack.size() >= 2) {
            long long sum = A[stack.back()->position];
            sum *= (long long) (stack.back()->leftDominance + 1);
            sum *= (long long) (stack.back()->rightDominance + 1);
            stack[stack.size()-2]->rightChildSum += sum + stack.back()->rightChildSum;
        }
        
        stack.pop_back();
    }
    
    
    nodes[n-1]->rAscendSequence = new Sequence(n-1);
    nodes[n-1]->lDNumSum = 0;
    nodes[n-1]->lDNumIdxSum = 0;
    stack.push_back(nodes[n-1]);
    
    lastNode = nullptr;
    
    for (int i = n-2; 0 <= i; i--) {
        vector<QueryNode*> popped;
        while (!stack.empty() && A[stack.back()->position] <= A[i]) {
            if (stack.size() >= 2) {
                long long sum = A[stack.back()->position];
                sum *= (long long) (stack.back()->leftDominance + 1);
                sum *= (long long) (stack.back()->rightDominance + 1);
                stack[stack.size()-2]->leftChildSum += sum + stack.back()->leftChildSum;
            }
            
            lastNode = stack.back();
            popped.push_back(stack.back());
            stack.pop_back();
        }
        
        // all nodes being added are a child node or a next node
        if (lastNode) {
            lastNode->rNext = nodes[i];
            nodes[i]->rAscendSequence = lastNode->rAscendSequence;
            nodes[i]->rAscendSequence->add(i);
            
            if (lastNode->position - 1 != i) {
                nodes[i]->rAscendSequence->breakpoints.insert(lastNode->position);
            }
            
            popped.pop_back(); // Removes lastNode
            while (!popped.empty()) {
                popped.back()->rAscendSequence->superior = nodes[i]->rAscendSequence;
                popped.pop_back();
            }
            
            nodes[i]->lDNumSum = lastNode->lDNumSum + lastNode->lDNum();
            nodes[i]->lDNumIdxSum = lastNode->lDNumIdxSum + lastNode->lDNumIdx();
            lastNode = nullptr;
        } else {
            stack[stack.size()-1]->rChild = nodes[i];
            nodes[i]->rAscendSequence = new Sequence(i);
            
            nodes[i]->lDNumSum = 0;
            nodes[i]->lDNumIdxSum = 0;
        }
        stack.push_back(nodes[i]);
    }
    
    while (!stack.empty()) {
        if (stack.size() >= 2) {
            long long sum = A[stack.back()->position];
            sum *= (long long) (stack.back()->leftDominance + 1);
            sum *= (long long) (stack.back()->rightDominance + 1);
            stack[stack.size()-2]->leftChildSum += sum + stack.back()->leftChildSum;
        }
        
        stack.pop_back();
    }
}


long long querySimple(int left, int right) {
    QueryNode**& nodes = QueryNode::nodes;
    
    long long sum = 0;
    
    if (QueryNode::A[left] > QueryNode::A[right]) {
        int next = left;
        while (next <= right) {
            sum += nodes[next]->getSum(left, right);
            next = nodes[next]->end()+1;
        }
    } else {
        int next = right;
        while (next >= left) {
            sum += nodes[next]->getRSum(left, right);
            next = nodes[next]->start()-1;
        }
    }
    return sum;
}

long long query(int left, int right);

long long ascendingSum(Sequence* s, int left, int right, int start, int stop) {
    QueryNode**& nodes = QueryNode::nodes;
    long long sum = 0;
    sum += nodes[stop]->singleSum(left, right);
    sum += (1 - left) * (nodes[stop]->rDNumSum - nodes[start]->rDNumSum);
    sum += nodes[stop]->rDNumIdxSum - nodes[start]->rDNumIdxSum;
    
    auto it = s->breakpoints.lower_bound(left);
    auto end = s->breakpoints.upper_bound(right);
    for (; it != end; ++it) {
        if (*it != stop) {
           sum += nodes[*it]->rightChildSum;
        } else if (nodes[*it]->end() <= right) {
            sum += nodes[*it]->rightChildSum;
            stop = nodes[*it]->end();
        }
    }
    
    sum += query(left, start-1);
    sum += query(stop+1, right);
    return sum;
}

long long rAscendingSum(Sequence* s, int left, int right, int start, int stop) {
    QueryNode**& nodes = QueryNode::nodes;
    long long sum = 0;
    sum += nodes[stop]->singleSum(left, right);
    sum += (right + 1) * (nodes[stop]->lDNumSum - nodes[start]->lDNumSum);
    sum -= nodes[stop]->lDNumIdxSum - nodes[start]->lDNumIdxSum;
    
    auto it = s->breakpoints.lower_bound(left);
    auto end = s->breakpoints.upper_bound(right);
    for (; it != end; ++it) {
        if (*it != stop) {
            sum += nodes[*it]->leftChildSum;
        } else if (nodes[*it]->start() >= left) {
            sum += nodes[*it]->leftChildSum;
            stop = nodes[*it]->start();
        }
    }
    
    // stop is to the left of start since it's reversed
    sum += query(left, stop-1);
    sum += query(start+1, right);
    
    return sum;
}

long long query(int left, int right) {
    if (right < left)
        return 0;
    
    if (right - left < 1000) {
        return querySimple(left, right);
    }
    QueryNode**& nodes = QueryNode::nodes;
    
    int max = getMax(left, right);
    Sequence* asc = nodes[max]->ascendSequence;
    Sequence* rAsc = nodes[max]->rAscendSequence;
    int first, last, rFirst, rLast;
    first = *(asc->elements.lower_bound(left));
    last = max;
    rFirst = *(--rAsc->elements.upper_bound(right));
    rLast = max;
    
    
    if (last - first > rFirst - rLast) {
        long long sum = ascendingSum(asc, left, right, first, last);
        return sum;
    } else {
        long long sum = rAscendingSum(rAsc, left, right, rFirst, rLast);
        return sum;
    }
    
}
    

int main()
{   
    int& n = QueryNode::n;
    long long*& A = QueryNode::A;
    
    int m;
    scanf("%d %d\n", &n, &m);
    
    A = new long long[n];
    for (int i = 0; i < n; i++) {
        scanf("%lld", &A[i]);
    }
    
    buildMaxFinder();
    buildNodes();
    
    int left, right;
    for (int i = 0; i < m; i++) {
        scanf("%d %d", &left, &right);
        printf("%lld\n", query(left-1, right-1));
    }
    return 0;
}
