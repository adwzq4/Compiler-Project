// Adam Wilson
// Compilers - P4
// 12/14/2020

#include <cstdio>
#include <string>
#include <cstdlib>
#include <iostream>
#include <stack>
#include "codeGen.hpp" 
#include "token.hpp"
#include "parser.hpp"
#include "node.hpp"
using namespace std;

stack <string> idStack, tempIDStack;
stack <int> varCountStack, outStack, inStack, tempVarStack;
int varCount = 0, tempNum = 0, outNum = 0, inNum = 0, numGlobals = 0;
char* targetName;
FILE * fp;

// create target file in append mode with the appropriate name, first clearing any file with that name
void createTargetFile(char* name) {
    fp = fopen(name, "w+");
	if (fp == NULL) { perror("oss: Error"); }
	fclose(fp);
    fp = fopen(name, "a");
    if (fp == NULL) {
        perror("codeGen: ");
        exit(-1);
    }
    targetName = name;
}

// print STOP, allocate storage for global and temporary variables, and close target file
void allocateStorage(){
    fprintf(fp, "STOP\n");
    int n = idStack.size();
    for (int i = 0; i < n; i++) {
        fprintf(fp, "%s 0\n", idStack.top().c_str());
        idStack.pop();
    }
    for (int i = 0; i < tempNum; i++) { fprintf(fp, "T%d 0\n", i); }
    fclose(fp);
    printf("Successfully created target file %s\n", targetName);
}

// traverses the parse tree in order, writing code to target file when it encounters any
// code-generating nodes
void traverseTree(struct Node* node) {
    int n = checkStaticSemantics(node);

    // process child 2, store ACC in a temp, put the tempNum on tempVarStack and increment,
    // process child 1, multiply or divide ACC by the temp at the top of tempVarStack, and pop
    if (node->nodeLabel == "expr") {
        if (node->nodeChildren.size() == 2) {
            traverseTree(node->nodeChildren[1]);
            fprintf(fp, "STORE T%d\n", tempNum);
            tempVarStack.push(tempNum);
            tempNum++;
            traverseTree(node->nodeChildren[0]);
            if (node->nodeTokens[0].instance == "/") { fprintf(fp, "DIV T%d\n", tempVarStack.top()); }
            else { fprintf(fp, "MULT T%d\n", tempVarStack.top()); }
            tempVarStack.pop();
        }
        else { traverseTree(node->nodeChildren[0]); }
    }
    // same handling as "expr" node, except add ADD or SUB statement to target according to node token
    else if (node->nodeLabel == "N") {
        if (node->nodeChildren.size() == 2) {
            traverseTree(node->nodeChildren[1]);
            fprintf(fp, "STORE T%d\n", tempNum);
            tempVarStack.push(tempNum);
            tempNum++;
            traverseTree(node->nodeChildren[0]);
            if (node->nodeTokens[0].instance == "+") { fprintf(fp, "ADD T%d\n", tempVarStack.top()); }
            else { fprintf(fp, "SUB T%d\n", tempVarStack.top()); }
            tempVarStack.pop();
        }
        else { traverseTree(node->nodeChildren[0]); }
    }
    // process child, and if there is a % token in the node, multiply ACC by -1
    else if (node->nodeLabel == "A") {
        traverseTree(node->nodeChildren[0]);
        if (!node->nodeTokens.empty()) { fprintf(fp, "MULT -1\n"); }
    }
    // if R node has an integer token or a global identifier token, load it into ACC; if it has a local
    // ID token, read from program stack at the position the ID was found in idStack; otherwise process child
    else if (node->nodeLabel == "R") {
        if (!node->nodeTokens.empty()) {
            if (n >= idStack.size() - numGlobals || n == -1) {
                fprintf(fp, "LOAD %s\n", node->nodeTokens[0].instance.c_str()); 
            }
            else { fprintf(fp, "STACKR %d\n", n); }
        }
        else { traverseTree(node->nodeChildren[0]); }
    }
    // if IN node has a global ID token, read into it in target; otherwise, read into a tempVar, load it into
    // ACC, increment tempNum, and write ACC to program stack at position the ID was found in idStack
    else if (node->nodeLabel == "in") {
        if (n >= idStack.size() - numGlobals) {
            fprintf(fp, "READ %s\n", node->nodeTokens[0].instance.c_str());
        }
        else {
            fprintf(fp, "READ T%d\n", tempNum);
            fprintf(fp, "LOAD T%d\n", tempNum);
            tempNum++;
            fprintf(fp, "STACKW %d\n", n);
        }
    }
    // process child, store ACC in tempVar, write tempVar to output, and increment tempNum
    else if (node->nodeLabel == "out") {
        traverseTree(node->nodeChildren[0]);
        fprintf(fp, "STORE T%d\n", tempNum);
        fprintf(fp, "WRITE T%d\n", tempNum);
        tempNum++;
    }
    // call conditional(), add out label to target with the label num at the top of outStack, then pop it
    else if (node->nodeLabel == "if") {
        conditional(node);
        fprintf(fp, "OUT%d: NOOP\n", outStack.top());
        outStack.pop();
    }
    // add in label to target, push current inNum to inStack and increment; call conditional(); in target,
    // branch to in label whose label num is at the top of inStack; pop inStack; then add out label to target
    // with the label num at the top of outStack; pop outStack
    else if (node->nodeLabel == "loop") {
        fprintf(fp, "IN%d: NOOP\n", inNum);
        inStack.push(inNum);
        inNum++;
        conditional(node);
        fprintf(fp, "BR IN%d\n", inStack.top());
        inStack.pop();
        fprintf(fp, "OUT%d: NOOP\n", outStack.top());
        outStack.pop();
    }
    // process child; if assign node has a global identifier, store ACC to it in target; otherwise, write
    // ACC to program stack in the position the ID was found in idStack
    else if (node->nodeLabel == "assign") {
        traverseTree(node->nodeChildren[0]);
        if (n >= idStack.size() - numGlobals) {
            fprintf(fp, "STORE %s\n", node->nodeTokens[0].instance.c_str());
        }
        else { fprintf(fp, "STACKW %d\n", n); }
    }
    else { 
        // recursive in-order traversal
        for (int i = 0; i < node->nodeChildren.size(); i++) { traverseTree(node->nodeChildren[i]); }

        // when leaving a block node, pop idStack and deallocate local storage in target x times each, 
        // where x is the top of varCountStack, then pop varCountStack once
        if (node->nodeLabel == "block") {
            for (int i = 0; i < varCountStack.top(); i++) { 
                idStack.pop(); 
                fprintf(fp, "POP\n");
            }
            varCountStack.pop();
        }
    }
}

// searches the top n entries in idStack for argument id, returning distance from TOS if found, otherwise -1
int find(string id, int n) {
    // transfer searched entries to tempStack
    int i;
    for (i = 0; i < n; i++) {
        string s = idStack.top();
        idStack.pop();
        tempIDStack.push(s);
        if (s == id) break;
    }

    // transfer tempStack back to idStack
    int k = tempIDStack.size();
    for (int j = 0; j < k; j++) {
        idStack.push(tempIDStack.top());
        tempIDStack.pop();
    }

    if (i == n) { return -1; }
    else { return i; }
}

// checks whether identifiers have already been declared in this scope in "vars" nodes,
// and in any other node checks whether identifers have been declared in any encompassing scope 
int checkStaticSemantics(struct Node* node) {
    if (node->nodeLabel == "vars") {
        if (!node->nodeTokens.empty()) {
            // if the instance is not found within varCount of TOS, push instance to idStack and increment varCount
            if (find(node->nodeTokens[0].instance, varCount) < 0) {
                idStack.push(node->nodeTokens[0].instance);
                // if non-global, allocate local storage on target stack
                if (varCountStack.size() > 0) { fprintf(fp, "PUSH\n"); }
                else { numGlobals++; }
                varCount++;
            }
            // otherwise display error message, line number, delete target file, and exit
            else {
                cout << "Error: '" << node->nodeTokens[0].instance;
                cout << "' was already defined in this scope (line " << node->nodeTokens[0].lineNum << ").\n";
                if (remove(targetName) != 0) { 
                    perror("codeGen: "); 
                    exit(-1);
                }
            }
        }

        // if an empty vars node is reached push varCount to varCountStack and reset varCount 
        else {
            varCountStack.push(varCount);
            varCount = 0;
        }

        return -1;
    }

    else {
        int n;
        for (int i = 0; i < node->nodeTokens.size(); i++) {
            // if the identifier is not found anywhere in idStack, display error and line #, delete target file, and exit
            n = find(node->nodeTokens[i].instance, idStack.size());
            if (node->nodeTokens[i].type == "identifier" && n < 0) {
                cout << "Error: '" << node->nodeTokens[i].instance;
                cout << "' is undefined (line " << node->nodeTokens[i].lineNum << ").\n";
                if (remove(targetName) != 0) { 
                    perror("codeGen: "); 
                    exit(-1);
                }
            }
        }
        return n;
    }
}

// handles the conditional part of both if and loop statements
void conditional(struct Node* node) {
    // process child 3, store ACC in a temp, put the tempNum on tempVarStack and increment,
    // process child 1, subtract the temp at the top of tempVarStack from ACC, and pop
    traverseTree(node->nodeChildren[2]);
    fprintf(fp, "STORE T%d\n", tempNum);
    tempVarStack.push(tempNum);
    tempNum++;
    traverseTree(node->nodeChildren[0]);
    fprintf(fp, "SUB T%d\n", tempVarStack.top());
    tempVarStack.pop();
    
    // add branch(es) to target according to the relational operator in the statement
    string s = node->nodeChildren[1]->nodeTokens[0].instance;
    if (s == "=<") { fprintf(fp, "BRPOS OUT%d\n", outNum); }
    else if (s == "=>") { fprintf(fp, "BRNEG OUT%d\n", outNum); }
    else if (s == "==") {
        fprintf(fp, "BRPOS OUT%d\n", outNum);
        fprintf(fp, "BRNEG OUT%d\n", outNum);
    }
    else if (s == ":") { fprintf(fp, "BRZERO OUT%d\n", outNum); }
    
    // push the current out label num onto outStack and increment
    outStack.push(outNum);
    outNum++;
    
    // process if/loop body
    traverseTree(node->nodeChildren[3]);
}