#ifndef __TREE_H__
#define __TREE_H__
#include"config.h"
#include "syntax.tab.h"

#include "symbol.h"
typedef struct Node
{
    int type;
    int line;
    int head;   //非终结符类型
    int expand; //产生式类型
    //表达式类型
    struct
    {
        Type* t;
        enum
        {
            LEFT,
            RIGHT
        } val;
    } exp;
    union
    {
        unsigned int i;
        float f;
        char c[50];
    };
    struct Node *firstChild, *nextSibling;
    FieldList *info;
    Type *last;
} Node;
#define NTERMINAL 0
void printt(Node *r,const int indentcnt);
Node *init(int type, int line, const char *text);
void insert(Node *f, Node *c);

//插入
bool insertStruct(Node *node);

enum
{
    Program,
    ExtDefList,
    ExtDef,
    ExtDecList,
    Specifier,
    StructSpecifier,
    OptTag,
    Tag,
    VarDec,
    FunDec,
    VarList,
    ParamDec,
    CompSt,
    StmtList,
    Stmt,
    DefList,
    Def,
    DecList,
    Dec,
    Exp,
    Args
};

#endif