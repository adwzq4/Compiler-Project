// Adam Wilson
// Compilers - P4
// 12/14/2020

#include <cstdio>
#include <string>
#include <cstdlib>
#include <iostream>
#include "token.hpp"
#include "scanner.hpp"
#include "parser.hpp"
#include "node.hpp"
using namespace std;

static struct token tk;

// prints the parse tree with a preorder traversal, with 3 spaces of indent per level of depth, printing each node
// label and any node tokens
void printParseTree(struct Node* node, int level) {
    int i;
	for (i = 0; i < level; i++) { cout << "   "; }
	cout << "<" << node->nodeLabel << ">";
	if (!node->nodeTokens.empty()) {
        cout << "   tokens: ";
        for (i = 0; i < node->nodeTokens.size() - 1; i++) { cout << "'" << node->nodeTokens[i].instance << "', "; }
        cout << "'" << node->nodeTokens[i].instance << "'";
    }
	cout << endl;

    if (!node->nodeChildren.empty()) {
        for (int i = 0; i < node->nodeChildren.size(); i++) { printParseTree(node->nodeChildren[i], level + 1); }
    }
}

// prints error message based on expected token and current token type/instance, then exits
void mError(string expected) {
    string received = tk.type;
    if (tk.type == "keyword" || tk.type == "operator/delimiter") { received = tk.instance; }
    cout << "Parser error: " << received << " tk received, " << expected << " tk expected (line " << tk.lineNum << ")" << endl;
    exit(1);
}

// auxilary process creates root of tree, gets first token and serves as base of recursive stack, then returns root
struct Node* parser() {
    struct Node* root;
    tk = scanner();
    root = program();
    if (tk.type == "EOF") { return root; }
    else { mError("EOF"); }
}

// handles <program> nonterminal: <program> -> start <vars> main <block> stop
struct Node* program() {
    struct Node* p = new Node("program");
    
    if (tk.instance == "start") {
        tk = scanner();
        p->nodeChildren.push_back(vars());
        if (tk.instance == "main") {
            tk = scanner();
            p->nodeChildren.push_back(block());
            if (tk.instance == "stop") { tk = scanner(); }
            else { mError("stop"); }
        }
        else { mError("main"); }
    }
    else { mError("start"); }
    
    return p;
}

// handles <block> nonterminal: <block> -> { <vars> <stats> }
struct Node* block() {
    struct Node* p = new Node("block");
    
    if (tk.instance == "{") {
        tk = scanner();
        p->nodeChildren.push_back(vars());
        p->nodeChildren.push_back(stats());
        if (tk.instance == "}") { tk = scanner(); }
        else { mError("}"); }
    }
    else { mError("{"); }
    
    return p;
}

// handles <vars> nonterminal: <vars> -> empty | let Identifier : Integer <vars>
struct Node* vars() {
    struct Node* p = new Node("vars");
    
    if (tk.instance == "let") {
        tk = scanner();
        if (tk.type == "identifier") {
            p->nodeTokens.push_back(tk);
            tk = scanner();
            if (tk.instance == ":") {
                tk = scanner();
                if (tk.type == "integer") {
                    p->nodeTokens.push_back(tk);
                    tk = scanner();
                    p->nodeChildren.push_back(vars());
                }
                else { mError("integer"); }
            }
            else { mError(":"); }
        }
        else { mError("identifier"); }
    }
    
    return p;
}

// handles <expr> nonterminal: <expr> -> <N> / <expr>  | <N> * <expr> | <N>
struct Node* expr() {
    struct Node* p = new Node("expr");
    
    p->nodeChildren.push_back(N());
    if (tk.instance == "/" || tk.instance == "*") {
        p->nodeTokens.push_back(tk);
        tk = scanner();
        p->nodeChildren.push_back(expr());
    }
    
    return p;
}

// handles <N> nonterminal: <N> -> <A> + <N> | <A> - <N> | <A>
struct Node* N() {
    struct Node* p = new Node("N");
    
    p->nodeChildren.push_back(A());
    if (tk.instance == "+" || tk.instance == "-") {
        p->nodeTokens.push_back(tk);
        tk = scanner();
        p->nodeChildren.push_back(N());
    }
    
    return p;
}

// handles <A> nonterminal: <A> -> % <A> |  <R>
struct Node* A() {
    struct Node* p = new Node("A");
    
    if (tk.instance == "%") {
        p->nodeTokens.push_back(tk);
        tk = scanner();
        p->nodeChildren.push_back(A());
    }
    else if (tk.instance == "[" || tk.type == "identifier" || tk.type == "integer") {
        p->nodeChildren.push_back(R());
    }
    else { mError("%, [, identifier, or integer"); }
    
    return p;
}

// handles <R> nonterminal: <R> -> [ <expr> ] | Identifier | Integer
struct Node* R() {
    struct Node* p = new Node("R");
    
    if (tk.instance == "[") {
        tk = scanner();
        p->nodeChildren.push_back(expr());
        if (tk.instance == "]") {
            tk = scanner();
        }
        else { mError("]"); }
    }
    else if (tk.type == "identifier" || tk.type == "integer") {
        p->nodeTokens.push_back(tk);
        tk = scanner();
    }
    else { mError("[, identifier, or integer"); }
    
    return p;
}

// handles <stats> nonterminal: <stats> -> <stat> <mStat>
struct Node* stats() {
    struct Node* p = new Node("stats");
    
    p->nodeChildren.push_back(stat());
    p->nodeChildren.push_back(mStat());
    
    return p;
}

// handles <mStat> nonterminal: <mStat> -> empty | <stat> <mStat>
struct Node* mStat() {
    struct Node* p = new Node("mStat");
    
    if (tk.instance == "scanf" || tk.instance == "printf" || tk.instance == "{" || 
        tk.instance == "if" || tk.instance == "iter" || tk.type == "identifier") {
        p->nodeChildren.push_back(stat());
        p->nodeChildren.push_back(mStat());
    }

    return p;
}

// handles <stat> nonterminal: <stat> -> <in> . | <out> . | <block> | <if> . | <loop> . | <assign> .
struct Node* stat() {
    struct Node* p = new Node("stat");
    
    if (tk.instance == "scanf") { p->nodeChildren.push_back(in()); } 
    else if (tk.instance == "printf") { p->nodeChildren.push_back(out()); } 
    else if (tk.instance == "{") { p->nodeChildren.push_back(block()); return p; } 
    else if (tk.instance == "if") { p->nodeChildren.push_back(if_()); } 
    else if (tk.instance == "iter") { p->nodeChildren.push_back(loop()); } 
    else if (tk.type == "identifier") { p->nodeChildren.push_back(assign()); }
    else { mError("scanf, printf, {, if, iter, or identifier"); }
    
    if (tk.instance == ".") { tk = scanner(); }
    else { mError("."); }
    
    return p;
}

// handles <in> nonterminal: <in> -> scanf [ Identifier ]
struct Node* in() {
    struct Node* p = new Node("in");
    
    if (tk.instance == "scanf") {
        tk = scanner();
        if (tk.instance == "[") {
            tk = scanner();
            if (tk.type == "identifier") {
                p->nodeTokens.push_back(tk);
                tk = scanner();
                if (tk.instance == "]") { tk = scanner(); }
                else { mError("]"); }
            }
            else { mError("identifer"); }
        }
        else { mError("["); }
    }
    else { mError("scanf"); }
                    
    return p;
}

// handles <out> nonterminal: <out> -> printf [ <expr> ]
struct Node* out() {
    struct Node* p = new Node("out");
    
    if (tk.instance == "printf") {
        tk = scanner();
        if (tk.instance == "[") {
            tk = scanner();
            p->nodeChildren.push_back(expr());
            if (tk.instance == "]") { tk = scanner(); }
            else { mError("]"); }
        }
        else { mError("["); }
    }
    else { mError("printf"); }
    
    return p;
}

// handles <if> nonterminal: <if> -> if [ <expr> <RO> <expr> ] then <block>
struct Node* if_() {
    struct Node* p = new Node("if");
    
    if (tk.instance == "if") {
        tk = scanner();
        if (tk.instance == "[") {
            tk = scanner();
            p->nodeChildren.push_back(expr());
            p->nodeChildren.push_back(RO());
            p->nodeChildren.push_back(expr());
            if (tk.instance == "]") {
                tk = scanner();
                if (tk.instance == "then") {
                    tk = scanner();
                    p->nodeChildren.push_back(block());
                }
                else { mError("then"); }
            }
            else { mError("]"); }
        }
        else { mError("["); }
    }
    else { mError("if"); }
    
    return p;
}

// handles <loop> nonterminal: <loop> -> iter [ <expr> <RO> <expr> ] <block>
struct Node* loop() {
    struct Node* p = new Node("loop");
    
    if (tk.instance == "iter") {
        tk = scanner();
        if (tk.instance == "[") {
            tk = scanner();
            p->nodeChildren.push_back(expr());
            p->nodeChildren.push_back(RO());
            p->nodeChildren.push_back(expr());
            if (tk.instance == "]") {
                tk = scanner();
                p->nodeChildren.push_back(block());
            }
            else { mError("]"); }
        }
        else { mError("["); }
    }
    else { mError("iter"); }

    return p;
}

// handles <assign> nonterminal: <assign> -> Identifier = <expr>
struct Node* assign() {
    struct Node* p = new Node("assign");
    
    if (tk.type == "identifier") {
        p->nodeTokens.push_back(tk);
        tk = scanner();
        if (tk.instance == "=") {
            tk = scanner();
            p->nodeChildren.push_back(expr());
        }
        else { mError("="); }
    }
    else { mError("identifier"); }

    return p;
}

// handles <RO> nonterminal: <RO> -> =< | => | == | : :
struct Node* RO() {
    struct Node* p = new Node("RO");
    
    if (tk.instance == "=<") {
        p->nodeTokens.push_back(tk);
        tk = scanner();
    }
    else if (tk.instance == "=>") {
        p->nodeTokens.push_back(tk);
        tk = scanner();
    }
    else if (tk.instance == "==") {
        p->nodeTokens.push_back(tk);
        tk = scanner();
    }
    else if (tk.instance == ":") {
        p->nodeTokens.push_back(tk);
        tk = scanner();
        if (tk.instance == ":") {
            p->nodeTokens.push_back(tk);
            tk = scanner();
        }
        else { mError(":"); }
    }
    else { mError("=<, =>, ==, or :"); }
    
    return p;
}
