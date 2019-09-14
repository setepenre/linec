%{
#include <iostream>
#include <string>

#include "node.hpp"
#include "parser.hpp"

#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;
extern "C" int yywrap() { return 1; }
%}

%option yylineno

integer [0-9]+
number  [0-9]*\.[0-9]+([eE][-+]?[0-9]+)?
text    \"(\\.|[^"\\])*\"
letters [a-zA-Z]+

%%
[ \t\r\n]+         {}
{integer}|{number} {
                       yylval.number = std::stod(yytext);
                       return T_NUMBER;
                   }
{text}             {
                       yylval.text = new std::string(yytext + 1, yyleng - 2);
                       return T_TEXT;
                   }
{letters}          {
                       yylval.name = new std::string(yytext);
                       return T_IDENTIFIER;
                   }
"="                return T_EQUAL;
.                  {
                       std::cerr << "Unknown token\n" << std::endl; 
                       yyterminate();
                   }
%%
