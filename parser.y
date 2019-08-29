%{
#include <iostream>

#include "node.hpp" 

Line* line;

extern int yylex();
void yyerror(const char* s) { std::cerr << "ERROR: " << s << std::endl; }
%}

%union {
    Line *line;
    Expr *expr;
    double number;
    int token;
}

%token <number> T_NUMBER 
%token <token> T_PLUS

%type <line> line
%type <expr> expr

%left T_PLUS

%%
line: 
    expr { line = new Line($1); }
    ;

expr:
    T_NUMBER           { $$ = new Number($1); }
    | T_PLUS expr expr { $$ = new Plus($2, $3); }
    ;
%%
