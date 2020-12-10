// Adam Wilson
// Compilers - P4
// 12/14/2020

// Compiles a .fs file into a .asm file based on following lexical definitions:
// Case sensitive
// Alphabet
//      all English letters (upper and lower), digits, special characters seen below, and WS
// Identifiers
//      begin with a  lower case letter
//      continue with any number of letters or digits, 8 significant total
// Keywords
//      start stop iter void int exit scanf printf main if then let data func
// Operators and delimiters group (all single character except ==  => and =<)
//      =  =>  =< ==  :   +  -  *  /   %  . (  ) , { } ; [ ]
// Integers
//      any sequence of decimal digits, no sign, no decimal point, up to 8 significant
// Comments start with # and end with end of the line

// And the following BNF:
// <program> ->   start <vars> main <block> stop
// <block>   ->   { <vars> <stats> }
// <vars>    ->   empty | let Identifier :  Integer    <vars>
// <expr>    ->   <N> / <expr>  | <N> * <expr> | <N>
// <N>       ->   <A> + <N> | <A> - <N> | <A>
// <A>       ->   % <A> |  <R>
// <R>       ->   [ <expr> ] | Identifier | Integer
// <stats>   ->   <stat>  <mStat>
// <mStat>   ->   empty |  <stat>  <mStat>
// <stat>    ->   <in> .  | <out> .  | <block> | <if> .  | <loop> .  | <assign> .  
// <in>      ->   scanf [ Identifier ]
// <out>     ->   printf [ <expr> ]
// <if>      ->   if [ <expr> <RO> <expr> ] then <block>
// <loop>    ->   iter [ <expr> <RO> <expr> ]  <block>
// <assign>  ->   Identifier  = <expr> 
// <RO>      ->   =<  | =>   |  ==  |   :  : 

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include "scanner.hpp"
#include "parser.hpp"
#include "codeGen.hpp" 
#include "node.hpp"
using namespace std;

// parses input text, either from stdio or from file specified in command line
int main(int argc, char* argv[]) {
    string input;
    char c;
    bool comment;
    char targetName[100];
    FILE* fp;

    // produces error message and exits if there are too many args
    if (argc > 2) {
        cout << "Error: too many command line arguments.\n";
        exit(-1);
    }

    // if a file name is given as a command line arg, extension ".fs" is appended, and corresponding
    // file is added char-by-char into a string, filtering out comments
    else if (argc == 2) {
        strcpy(targetName, argv[1]);
        fp = fopen(strcat(argv[1], ".fs"), "r");
        if (fp != NULL) {
            for (c = getc(fp); c != EOF; c = getc(fp)) {
                if (c == '#') comment = true;
                else if (comment && c == '\n') comment = false;
                if (!comment) input += c;
            }
            fclose(fp);
            strcat(targetName, ".asm");
        }
        else {
            perror("main: ");
            exit(-1);
        }
    }

    // otherwise reads stdin, either from keyboard until simulated EOF or from redirected
    // file, into a string while filtering out comments
    else {
        strcpy(targetName, "kb.asm");
        for (c = getchar(); c != EOF; c = getchar()) {
            if (c == '#') { comment = true; }
            else if (comment && c == '\n') { comment = false; }
            if (!comment) { input += c; }
        }
    }

    // initializes scanner.cpp with preprocessed input string
    initializeScanner(input);

    // create recursive descent parse tree from source code
    struct Node* parseTreeRoot = parser();

    // generate code in target file while checking static semantics, then allocate globals storage
    createTargetFile(targetName);
    traverseTree(parseTreeRoot);
    allocateStorage();
}
