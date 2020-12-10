// Adam Wilson
// Compilers - P4
// 12/14/2020

#include <cstdio>
#include <string>
#include <utility> 
#include <iostream>
#include <cstdlib>
#include <cctype>
#include <algorithm>
#include "token.hpp"
#include "scanner.hpp"
using namespace std;

static int index = 0;
static int lineNum = 1;
static string text;

// assigns different types of characters to column numbers from the FSA table
int columnAssigner(char c) {
	// array of valid operators and delimiters
	char ops[] = { ':', '+', '-', '*', '/', '%', '.', '(', ')', ',', '{', '}', ';', '[', ']' };
	if (isspace(c)) { return 0; }									// WS -> column 0
	else if (isupper(c)) { return 1; }								// uppercase -> column 1
	else if (islower(c)) { return 2; }								// lowercase -> column 2
	else if (isdigit(c)) { return 3; }								// digits -> column 3
	else if (c == '=') { return 4; }								// = -> column 4
	else if (c == '<') { return 5; }								// < -> column 5
	else if (c == '>') { return 6; }								// > -> column 6
	else if (find(ops, ops + 15, c) != ops + 15) { return 7; }		// other allowed ops -> column 7
	else { return 9; }												// non-allowed characters -> column 9
}

// put pre-processed input into local variable
void initializeScanner(string processedInput) { text = processedInput; }

// scans characters, changing states accordingly until a token is formed, then returns token struct
struct token scanner() {
	state currentState = s0;
	pair <state, string> nextState;
	string lex ("");
	int column;

	// processes input according to FSA table
	while (currentState != fin) {
		// assigns a column number to the currect char, incrementing lineNum upon newline
		if (index < text.length()) { column = columnAssigner(text[index]); }
		else { column = 8; }

		nextState = FSA[currentState][column];

		// outputs error message upon error state
		if (nextState.first == error) {
			printf("Scanner error: %s\n", nextState.second.c_str());
			exit(-1);
		}
		
		// upon final state, and if token is an identifier, checks it against list of keywords to
		// see if token is a keyword, then returns appropraite token type and token instance
		else if (nextState.first == fin) {
			if (nextState.second == "identifier") {
				for (int i = 0; i < NUMKEYWORDS; i++) {
					if (lex == keywords[i]) { return (struct token) { string("keyword"), lex, lineNum }; }
				}
				return (struct token) { string("identifier"), lex, lineNum };
			}
			return (struct token) { nextState.second, lex, lineNum };
		}

		// if in a non-terminal state, appends curent char (unless lex.length() is >= 8) then increments index
		else {
			currentState = nextState.first;
			if (!isspace(text[index]) && lex.length() < 8) { lex += text[index]; }
			if (text[index] == '\n') { lineNum++; }
			index++;
		}
	}
}

// calls scanner() until EOf is reached, outputs info for each token, and increments lineNum appropriately
void testScanner() {
	struct token tk = scanner();
	while (tk.type != string ("EOF")) {
		printf("Token type: %-20s Instance: %-10s Line: %d\n", tk.type.c_str(), tk.instance.c_str(), tk.lineNum);
		tk = scanner();
	}
	printf("Token type: %-20s Instance: %-10s Line: %d\n", tk.type.c_str(), tk.instance.c_str(), tk.lineNum);
	index = 0; lineNum = 1;
}
