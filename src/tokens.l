%{
#include <iostream>
#include <string>

#include "node.hpp"
#include "parser.hpp"

extern "C" int yywrap() { return 1; }
%}

%option yylineno

integer [0-9]+
number  [0-9]*\.[0-9]+([eE][-+]?[0-9]+)?
letters [a-zA-Z]+
%%
[ \t\r\n]+         {}
{integer}|{number} {
                       yylval.number = std::stod(yytext);
                       return T_NUMBER;
                   }
{letters}          {
                       yylval.name = new std::string(yytext);
                       return T_IDENTIFIER;
                   }
"="                return T_EQUAL;
"+"                return T_PLUS;
.                  {
                       std::cerr << "Unknown token\n" << std::endl; 
                       yyterminate();
                   }
%%
