// Adam Wilson
// Compilers - P4
// 12/14/2020

#ifndef NODE_HPP
#define NODE_HPP
#include <string>
#include <vector>
#include "token.hpp"
using namespace std;

// node struct for Recursive Descent Tree
struct Node {
    string nodeLabel;
    vector<token> nodeTokens;
    vector<Node*> nodeChildren;

    Node (string lbl) {
        nodeLabel = lbl;
    }
};

#endif
