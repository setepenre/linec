%{
#include <iostream>
#include <string>

#include "node.hpp"
#include "parser.hpp"

extern "C" int yywrap() { return 1; }
%}

integer [0-9]+
number  [0-9]*\.[0-9]+([eE][-+]?[0-9]+)?
%%
[ \t\n]            ;
{integer}|{number} {
                       yylval.number = std::stod(yytext);
                       return T_NUMBER;
                   }
"+"                return T_PLUS;
.                  {
                       std::cerr << "Unknown token\n" << std::endl; 
                       yyterminate();
                   }
%%
