// Adam Wilson
// Compilers - P4
// 12/14/2020

#include <string>
#include "token.hpp"
using namespace std;

// define token and error types
string tokens[5] = { "identifier", "integer", "operator/delimiter", "keyword", "EOF" };
string errors[4] = { "no token begins with an uppercase letter", "no token begins with <", "no token begins with >", "invalid character" };
string keywords[14] = { "start", "stop", "iter", "void", "int", "exit", "scanf", "printf", "main", "if", "then", "let", "data", "func" };
int NUMKEYWORDS = 14;

// FSA table
pair <state, string> FSA[][10] = {
	{make_pair(s0, string("")), make_pair(error, errors[0]), make_pair(s1, string("")), make_pair(s2, string("")), make_pair(s3, string("")), make_pair(error, errors[1]), make_pair(error, errors[2]), make_pair(s4, string("")), make_pair(fin, tokens[4]), make_pair(error, errors[3])},
	{make_pair(fin, tokens[0]), make_pair(s1, string("")), make_pair(s1, string("")), make_pair(s1, string("")), make_pair(fin, tokens[0]), make_pair(fin, tokens[0]), make_pair(fin, tokens[0]), make_pair(fin, tokens[0]), make_pair(fin, tokens[0]), make_pair(error, errors[3])},
	{make_pair(fin, tokens[1]), make_pair(fin, tokens[1]), make_pair(fin, tokens[1]), make_pair(s2, string("")), make_pair(fin, tokens[1]), make_pair(fin, tokens[1]), make_pair(fin, tokens[1]), make_pair(fin, tokens[1]), make_pair(fin, tokens[1]), make_pair(error, errors[3])},
	{make_pair(fin, tokens[2]), make_pair(fin, tokens[2]), make_pair(fin, tokens[2]), make_pair(fin, tokens[2]), make_pair(s4, string("")), make_pair(s4, string("")), make_pair(s4, string("")), make_pair(fin, tokens[2]), make_pair(fin, tokens[2]), make_pair(error, errors[3])},
	{make_pair(fin, tokens[2]), make_pair(fin, tokens[2]), make_pair(fin, tokens[2]), make_pair(fin, tokens[2]), make_pair(fin, tokens[2]), make_pair(fin, tokens[2]), make_pair(fin, tokens[2]), make_pair(fin, tokens[2]), make_pair(fin, tokens[2]), make_pair(error, errors[3])}
};