// Adam Wilson
// Compilers - P4
// 12/14/2020

#ifndef SCANNER_HPP
#define SCANNER_HPP
#include <utility>
#include <string>
using namespace std;

int columnAssigner(char);
struct token scanner();
void initializeScanner(string);
void testScanner();

#endif
