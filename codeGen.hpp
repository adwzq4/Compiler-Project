// Adam Wilson
// Compilers - P4
// 12/14/2020

#ifndef CODEGEN_HPP
#define CODEGEN_HPP
#include <cstdio>
#include <string>
#include <cstdlib>
#include <iostream>
#include <stack> 
using namespace std;

void createTargetFile(char*);
void allocateStorage();
void traverseTree(struct Node*);
int find(string, int);
int checkStaticSemantics(struct Node*);
void conditional(struct Node*);

#endif