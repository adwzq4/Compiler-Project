// Adam Wilson
// Compilers - P4
// 12/14/2020

#ifndef TOKEN_HPP
#define TOKEN_HPP
#include <string>
#include <vector>
using namespace std; 

struct token {
    string type;
    string instance;
    int lineNum;
};

// enumerates possibles states
enum state { s0, s1, s2, s3, s4, fin, error };

// declare FSA table
extern pair<state, string> FSA[][10];

// declare token and error types
extern string tokens[];
extern string errors[];
extern string keywords[];
extern int NUMKEYWORDS;

#endif
