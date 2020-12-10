// Adam Wilson
// Compilers - P4
// 12/14/2020

#ifndef PARSER_HPP
#define PARSER_HPP
#include <string>
using namespace std;

void printParseTree(struct Node*, int);
void mError(string);
struct Node* parser();
struct Node* program();
struct Node* block();
struct Node* vars();
struct Node* expr();
struct Node* N();
struct Node* A();
struct Node* R();
struct Node* stats();
struct Node* mStat();
struct Node* stat();
struct Node* in();
struct Node* out();
struct Node* if_();
struct Node* loop();
struct Node* assign();
struct Node* RO();

#endif