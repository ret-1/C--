%locations
%define api.pure
%define parse.error verbose
//%pure-parser

%{
#include "tree.h"
#define YYSTYPE Node*
#include "lex.yy.c"
#include <stdio.h>
#include <assert.h>
#include "config.h"
//Node* root;
void yyerror(const char *msg);
Node* root;
int syntax_e=0;


void check(int idx,Node* root, Node** vsp, int n);

#define ACT(name, n, idx, e) \
yyval = init(NTERMINAL, yyloc.first_line, #name); \
yyval->head=name; \
yyval->expand = e; \
for(int i = 1; i <= n; i++) \
    insert((yyval),(yyvsp[(i) - (n)]));\
check(idx, yyval, yyvsp, n);

#define ERROR \
syntax_e = 1; \
yyerrok;

#define ACT_NULL yyval = 0;


const char* typename[28] = {"INT","FLOAT","ID","SEMI","COMMA","ASSIGNOP","RELOP",
                            "PLUS","MINUS","STAR","DIV","AND","OR","DOT",
                            "NOT","TYPE","LP","RP","LB","RB","LC",
                            "RC","STRUCT","RETURN","IF","ELSE","WHILE","UMINUS"};

int inStruct=0;
FieldList* tType=NULL;
%}

%token INT FLOAT ID
%token SEMI COMMA
%token ASSIGNOP RELOP
%token PLUS MINUS STAR DIV
%token AND OR DOT NOT TYPE
%token LP RP LB RB LC RC
%token STRUCT RETURN IF ELSE WHILE


%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right UMINUS
%right NOT
%left LB RB LP RP DOT

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE


%%
//High-level Definitions
Program: ExtDefList                             {ACT(Program,1,-1,1) root=yyval;};
ExtDefList: ExtDef ExtDefList                   {ACT(ExtDefList,2,-1,1)}
    |                                           {ACT_NULL};
ExtDef: Specifier ExtDecList SEMI               {ACT(ExtDef,3,19,1)}
    | Specifier SEMI                            {ACT(ExtDef,2,28,2)}
    | Specifier FunDec {check(27, yyval, yyvsp, 2);} CompSt
                                                {yyval = init(NTERMINAL, yyloc.first_line, "ExtDef");
                                                yyval->head=ExtDef;
                                                yyval->expand = 3;
                                                insert(yyval,$1);insert(yyval,$2);insert(yyval,$4);
                                                if (tType) tType = tType->tail;}
    | error SEMI                                {ERROR}
    | error CompSt                              {ERROR};
ExtDecList: VarDec                              {ACT(ExtDecList,1,29,1)}
    | VarDec COMMA ExtDecList                   {ACT(ExtDecList,3,30,2)};

//Specifiers
Specifier: TYPE                                 {ACT(Specifier,1,26,1)}
    | StructSpecifier                           {ACT(Specifier,1,25,2)};
StructSpecifier: STRUCT OptTag LC {inStruct++;} DefList {inStruct--;} RC    
                                                {yyval = init(NTERMINAL, yyloc.first_line, "StructSpecifier");
                                                yyval->head=StructSpecifier;
                                                yyval->expand = 1;
                                                for(int i = 1; i <= 7; i++)
                                                    if(i!=4 && i!=6) 
                                                        insert((yyval),(yyvsp[(i) - (7)]));
                                                check(24, yyval, yyvsp, 7);}
    | STRUCT Tag                                {ACT(StructSpecifier,2,23,2)};
OptTag: ID                                      {ACT(OptTag,1,-1,1)}
    |                                           {ACT_NULL};
Tag: ID                                         {ACT(Tag,1,-1,1)};

//Declarators
VarDec: ID                                      {ACT(VarDec,1,12,1)}
    | VarDec LB INT RB                          {ACT(VarDec,4,13,2)};
FunDec: ID LP VarList RP                        {ACT(FunDec,4,16,1)}
    | ID LP RP                                  {ACT(FunDec,3,16,2)}
    | ID LP error RP                            {ERROR};
VarList: ParamDec COMMA VarList                 {ACT(VarList,3,15,1)}
    | ParamDec                                  {ACT(VarList,1,15,2)};
ParamDec: Specifier VarDec                      {ACT(ParamDec,2,14,1)};

//Statements
CompSt: LC DefList StmtList RC                  {ACT(CompSt,4,-1,1)}
    | LC DefList error StmtList RC              {ERROR};
StmtList: Stmt StmtList                         {ACT(StmtList,2,-1,1)}
    |                                           {ACT_NULL};
Stmt: Exp SEMI                                  {ACT(Stmt,2,-1,1)}
    | CompSt                                    {ACT(Stmt,1,-1,2)}
    | RETURN Exp SEMI                           {ACT(Stmt,3,22,3)}
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE   {ACT(Stmt,5,21,4)}
    | IF LP Exp RP Stmt ELSE Stmt               {ACT(Stmt,7,21,5)}
    | WHILE LP Exp RP Stmt                      {ACT(Stmt,5,21,6)}
    | error SEMI                                {ERROR}
    | Exp error                                 {ERROR}
    | Exp error SEMI                            {ERROR}
    | RETURN Exp error SEMI                     {ERROR}
    | RETURN error SEMI                         {ERROR}
    | error RP Stmt                             {ERROR}
    | error RP Stmt ELSE Stmt                   {ERROR};

//Local Definitions
DefList: Def DefList                            {ACT(DefList,2,20,1)}
    |                                           {ACT_NULL};
Def: Specifier DecList SEMI                     {ACT(Def,3,19,1)}
    | Specifier error SEMI                      {ERROR};
DecList: Dec                                    {ACT(DecList,1,18,1)}
    | Dec COMMA DecList                         {ACT(DecList,3,18,2)};
Dec: VarDec                                     {ACT(Dec,1,17,1)}
    | VarDec ASSIGNOP Exp                       {ACT(Dec,3,17,2)};

//Expressions
Exp: Exp ASSIGNOP Exp                           {ACT(Exp,3,10,1)}
    | Exp AND Exp                               {ACT(Exp,3,9,2)}
    | Exp OR Exp                                {ACT(Exp,3,9,3)}
    | Exp RELOP Exp                             {ACT(Exp,3,8,4)}
    | Exp PLUS Exp                              {ACT(Exp,3,8,5)}
    | Exp MINUS Exp                             {ACT(Exp,3,8,6)}
    | Exp STAR Exp                              {ACT(Exp,3,8,7)}
    | Exp DIV Exp                               {ACT(Exp,3,8,8)}
    | LP Exp RP                                 {ACT(Exp,3,7,9)}
    | MINUS Exp %prec UMINUS                    {ACT(Exp,2,6,10)}
    | NOT Exp                                   {ACT(Exp,2,5,11)}
    | ID LP Args RP                             {ACT(Exp,4,4,12)}
    | ID LP RP                                  {ACT(Exp,3,4,13)}
    | Exp LB Exp RB                             {ACT(Exp,4,3,14)}
    | Exp DOT ID                                {ACT(Exp,3,2,15)}
    | ID                                        {ACT(Exp,1,1,16)}
    | INT                                       {ACT(Exp,1,0,17)}
    | FLOAT                                     {ACT(Exp,1,0,18)};

Args: Exp COMMA Args                            {ACT(Args,3,11,1)}
    | Exp                                       {ACT(Args,1,11,2)};

%%
void yyerror(const char* msg){
    printf("Error type B at Line %d: %s.\n", yylineno, msg);
    //printf("Error Type B at Line %d\n", yylineno);
}