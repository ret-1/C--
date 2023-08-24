%option bison-locations yylineno
%{
#include <stdlib.h>
#include <assert.h>
#include "tree.h"
#include "config.h"

int lexical_e=0;
int yycolumn = 1; 
#define YY_USER_ACTION \
yylloc->first_line = yylloc->last_line = yylineno; \
yylloc->first_column = yycolumn; \
yylloc->last_column = yycolumn + yyleng - 1; \
yycolumn += yyleng;

//#define YY_(msgid) &yylloc, msgid
//.                   {lexical_e=1; printf("Error type A at Line %d: Mysterious character \"%s\".\n", yylloc->first_line, yytext);}
#define YYLEX_PARAM &yylval, &yylloc

#define INIT(t) \
*yylval = init(t, yylineno, yytext); \
return t;

%}
delim   [ \t\n]
ws      {delim}+
letter  [_a-zA-Z]
integer 0|[1-9][0-9]*
digit16 [0-9a-fA-F]
id      {letter}({letter}|[0-9])*
%%
struct              {INIT(STRUCT);}
return              {INIT(RETURN);}
if                  {INIT(IF);}
else                {INIT(ELSE);}
while               {INIT(WHILE);}
int|float           {INIT(TYPE);}
{integer}           {INIT(INT);}
0[0-7]*             {INIT(INT);}
0[xX]{digit16}+     {INIT(INT);}
{integer}\.[0-9]+   {INIT(FLOAT);}
{id}                {INIT(ID);}
";"                 {INIT(SEMI);}
","                 {INIT(COMMA);}
"="                 {INIT(ASSIGNOP);}
">"                 {INIT(RELOP);}
"<"                 {INIT(RELOP);}
">="                {INIT(RELOP);}
"<="                {INIT(RELOP);}
"=="                {INIT(RELOP);}
"!="                {INIT(RELOP);}
"+"                 {INIT(PLUS);}
"-"                 {INIT(MINUS);}
"*"                 {INIT(STAR);}
"/"                 {INIT(DIV);}
"&&"                {INIT(AND);}
"||"                {INIT(OR);}
"."                 {INIT(DOT);}
"!"                 {INIT(NOT);}
"("                 {INIT(LP);}
")"                 {INIT(RP);}
"["                 {INIT(LB);}
"]"                 {INIT(RB);}
"{"                 {INIT(LC);}
"}"                 {INIT(RC);}
\n                  {yycolumn = 1; }
{ws}                {}
.                   {lexical_e=1; printf("Error type A at Line %d: Mysterious character \"%s\".\n", yylloc->first_line, yytext);}
%%