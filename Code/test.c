#include"config.h"
#ifdef DEBUG3
#include"ir.h"
#include<stdlib.h>
#include<stdio.h>
int main(int argc, char **argv){
    IChead = (InterCode*)malloc(sizeof(InterCode));
    InterCode* t = (InterCode*)malloc(sizeof(InterCode));
    InterCode* t2 = (InterCode*)malloc(sizeof(InterCode));
    Operand* arg1 = (Operand*)malloc(sizeof(Operand));
    Operand* arg2 = (Operand*)malloc(sizeof(Operand));
    arg1->kind = VARIABLE;
    arg1->value = 0;
    arg2->kind = CONSTANT;
    arg2->value = 10;
    t->kind = ASSIGN;
    t->arg1 = arg1;
    t->arg2 = arg2;
    t2->arg1 = arg1;
    t2->arg2 = arg2;
    t2->kind = ASSIGN;
    insertIC(t);
    insertIC(t2);
    InterCodeoutput(argv[1]);
    return 0;
}

#endif