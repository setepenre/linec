%{
#include <iostream>
#include <string>

#include "node.hpp" 

Block* entry;

extern int yylex();
extern int yylineno;
void yyerror(const char* s) { std::cerr << "ERROR: " << s << std::endl; }
%}

%union {
    Block* block;
    Expr* expr;
    std::string* name;
    std::string* text;
    double number;
    int token;
}

%token <name> T_IDENTIFIER
%token <text> T_TEXT
%token <number> T_NUMBER 
%token <token> T_EQUAL 

%type <block> program stmts
%type <expr> stmt expr

%%
program: /* empty */ {}
    | stmts { entry = $1; }
    ;

stmts: 
    stmt { $$ = new Block(yylineno, $1); }
    | stmts stmt { $1->push_back($2); }
    ;

stmt:
    T_IDENTIFIER T_EQUAL expr { $$ = new Assignment(yylineno, *$1, $3); }
    | expr { $$ = $1; }
    ;

expr:
    T_NUMBER           { $$ = new Number(yylineno, $1); }
    | T_TEXT           { $$ = new String(yylineno, *$1); }
    | T_IDENTIFIER     { $$ = new Ident(yylineno, *$1); }
    ;
%%
