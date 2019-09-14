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
    Block *block;
    Expr *expr;
    std::string *name;
    double number;
    int token;
}

%token <name> T_IDENTIFIER
%token <number> T_NUMBER 
%token <token> T_EQUAL T_PLUS

%type <block> program stmts
%type <expr> stmt expr

%left T_PLUS

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
    | T_IDENTIFIER     { $$ = new Ident(yylineno, *$1); }
    | T_PLUS expr expr { $$ = new Plus(yylineno, $2, $3); }
    ;
%%
