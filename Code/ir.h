#ifndef __IR_H__
#define __IR_H__
#include "tree.h"
typedef struct Operand Operand;
typedef struct InterCode InterCode;

int varCount;

struct Operand{
    enum
    {
        VARIABLE,
        CONSTANT,
        GETADDR, //取地址 &
        LABEL,
        FUN,
        GETVAL, //取值 *
        TEMP
    } kind;
    int value; //变量，函数，标签为序号,常量为值,取地址或取值为存地址的变量的序号
    int size;
    Operand *next;
};

struct InterCode{
    enum
    {
        ASSIGN, // left := right
        ADD, // result := op1 +-*/ op2
        SUB,
        MUL,
        DIVI,
        FUNC,
        PARAM,
        DEC,
        RET,
        LAB,
        GOTO,
        CALL, 
        ARG,
        READ,
        WRITE,
        IFGO,
    } kind;
    Operand *arg1, *arg2, *arg3;
    char* name;//存relop具体符号
    InterCode *prev, *next;
    bool skip;
};

InterCode* IChead;

// 生成中间代码
InterCode *initIC(int k, Operand *arg1, Operand *arg2, Operand *arg3);

//将root接到中间代码结果上
void insertIC(InterCode *root);

// 生成操作数
Operand *initOp(int k);

void translate(Node *root, Operand *arg1, Operand *arg2);

void Exp_Cond(Node *root, Operand *tl, Operand *fl);

//输出中间代码，参数为输出文件名
void InterCodeoutput(char* filename);

char *operand2string(Operand *op);
#endif