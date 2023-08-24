#include "ir.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define IR(p) translate(p, 0, 0);

int funCount = 0;
int labelCount = 0;
int optE = 0;
Operand *zero, *one;
Operand *mainfun = NULL;

void error(){
    printf("Cannot translate: Code contains variables of multi-dimensional array type or parameters of array type\n");
    exit(-1);
}

void translate(Node *root, Operand *arg1, Operand *arg2)
{
    if (!root)
        return;
    FieldList *p, *q;
    Operand *op1, *op2, *op3, *op4, *op5,*op6;
    InterCode *ic1, *ic2, *ic3;
    Node *r;
    //TODO:相同分支合并
    switch (root->head)
    {
    case Program:
        //TODO:初始化
        IChead = (InterCode *)malloc(sizeof(InterCode));
        varCount = 0;
        zero = initOp(CONSTANT);
        zero->value = 0;
        one = initOp(CONSTANT);
        one->value = 1;
        IR(root->firstChild)
        break;
    case ExtDefList: // ExtDefList -> ExtDef ExtDefList
        IR(root->firstChild)
        IR(root->firstChild->nextSibling)
        break;
    case ExtDef: //ExtDef -> Specifier FunDec CompSt
        //没有全局变量，因此其他产生式不需要处理
        if (root->expand == 3)
        {
            //IR(root->firstChild->nextSibling)
            p = root->firstChild->nextSibling->info;
            //function : ...
            op1 = initOp(FUN);
            //单独标记main函数
            if(strcmp(p->name,"main") == 0){
                mainfun = op1;
            }
            // 符号表对应项记录
            q = findSymbol(p->name);
            q->op = op1;
            insertIC(initIC(FUNC, op1, 0, 0));
            // PARAM ...
            p = p->type->u.function.params;
            while (p)
            {
                q = findSymbol(p->name);
                if(q->type->kind == ARRAY){
                    error();
                }
                op2 = initOp(VARIABLE);
                q->op = op2;
                q->param = true;
                ic2 = initIC(PARAM, op2, 0, 0);
                insertIC(ic2);
                p = p->next;
            }
            // CompSt
            IR(root->firstChild->nextSibling->nextSibling)
        }
        break;
    case ExtDecList:
        //没有全局变量，不需要处理
        break;
    case Specifier:
        break;
    case StructSpecifier:
        break;
    case OptTag:
        break;
    case Tag:
        break;
    case VarDec:
        break;
    case FunDec:
        //已处理
        break;
    case VarList:
        break;
    case ParamDec:
        break;
    case CompSt: //CompSt -> LC DefList StmtList RC
        IR(root->firstChild->nextSibling)
        if (root->firstChild->nextSibling->nextSibling->type == NTERMINAL)
            IR(root->firstChild->nextSibling->nextSibling)
        break;
    case StmtList: // StmtList -> Stmt StmtList
        IR(root->firstChild)
        IR(root->firstChild->nextSibling)
        break;
    case Stmt:
        switch (root->expand)
        {
        case 1:
        case 2: // Exp SEMI // CompSt
            IR(root->firstChild)
            break;
        case 3: // RETURN Exp
            op1 = initOp(TEMP);
            translate(root->firstChild->nextSibling, op1, 0);
            insertIC(initIC(RET, op1, 0, 0));
            break;
        case 4: // IF LP Exp RP Stmt
            op1 = initOp(LABEL);
            op2 = initOp(LABEL);
            Exp_Cond(root->firstChild->nextSibling->nextSibling, op1, op2);
            insertIC(initIC(LAB, op1, 0, 0));
            IR(root->firstChild->nextSibling->nextSibling->nextSibling->nextSibling)
            insertIC(initIC(LAB, op2, 0, 0));
            break;
        case 5: // IF LP Exp RP Stmt ELSE Stmt
            op1 = initOp(LABEL);
            op2 = initOp(LABEL);
            op3 = initOp(LABEL);
            Exp_Cond(root->firstChild->nextSibling->nextSibling, op1, op2);
            insertIC(initIC(LAB, op1, 0, 0));
            IR(root->firstChild->nextSibling->nextSibling->nextSibling->nextSibling)
            insertIC(initIC(GOTO, op3, 0, 0));
            insertIC(initIC(LAB, op2, 0, 0));
            IR(root->firstChild->nextSibling->nextSibling->nextSibling->nextSibling->nextSibling->nextSibling)
            insertIC(initIC(LAB, op3, 0, 0));
            break;
        case 6: //  WHILE LP Exp RP Stmt
            op1 = initOp(LABEL);
            op2 = initOp(LABEL);
            op3 = initOp(LABEL);
            insertIC(initIC(LAB, op1, 0, 0));
            Exp_Cond(root->firstChild->nextSibling->nextSibling, op2, op3);
            insertIC(initIC(LAB, op2, 0, 0));
            IR(root->firstChild->nextSibling->nextSibling->nextSibling->nextSibling)
            insertIC(initIC(GOTO, op1, 0, 0));
            insertIC(initIC(LAB, op3, 0, 0));
            break;
        default:
            break;
        }
        break;
    case DefList: // DefList -> Def DefList
        //IR(root->firstChild)
        //IR(root->firstChild->nextSibling)
        p = root->info;
        for (; p; p = p->next)
        {
            q = findSymbol(p->name);
            op1 = initOp(VARIABLE);
            q->op = op1;
            if (q->type->kind != BASIC)
            {
                op1->size = TypeSize(q->type);
                insertIC(initIC(DEC, op1, 0, 0));
            }
        }
        r = root->firstChild;
        while (r)
        {
            IR(r)
            if (r->nextSibling)
                r = r->nextSibling->firstChild;
            else
                break;
        }
        break;
    case Def: //Def -> Specifier DecList SEMI
        IR(root->firstChild->nextSibling)
        break;
    case DecList: // DecList -> Dec | Dec COMMA DecList
        IR(root->firstChild)
        if (root->expand == 2)
            IR(root->firstChild->nextSibling->nextSibling)
        break;
    case Dec: // Dec -> VarDec | VarDec = Exp
        if (root->expand == 2)
        {
            p = findSymbol(root->info->name);
            op1 = (Operand *)(p->op);
            op2 = initOp(TEMP);
            translate(root->firstChild->nextSibling->nextSibling, op2, 0);
            insertIC(initIC(ASSIGN, op1, op2, 0));
        }
        break;
    case Exp:
        switch (root->expand)
        {
        case 1: // Exp = Exp
            op1 = initOp(GETVAL);
            if (root->firstChild->exp.t->kind == ARRAY) //数组赋值
            {
                op2 = initOp(GETVAL);
                translate(root->firstChild->nextSibling->nextSibling, op2, 0);
                translate(root->firstChild, op1, 0);
                op4 = initOp(CONSTANT);
                op4->value = 4; //只能是int,4
                Operand *count = initOp(TEMP);
                op3 = initOp(LABEL);
                Operand *last = initOp(LABEL);
                Operand *_op1 = (Operand *)malloc(sizeof(Operand));
                Operand *_op2 = (Operand *)malloc(sizeof(Operand));
                if (op1->kind == TEMP || op1->kind == VARIABLE)
                    _op1->kind = TEMP;
                else
                    _op1->kind = GETADDR;
                _op1->value = op1->value;
                if (op2->kind == TEMP || op2->kind == VARIABLE)
                    _op2->kind = TEMP;
                else
                    _op2->kind = GETADDR;
                _op2->value = op2->value;
                Operand *size1 = initOp(CONSTANT);
                Operand *size2 = initOp(CONSTANT);
                size1->value = root->firstChild->exp.t->u.array.size;
                size2->value = root->firstChild->nextSibling->nextSibling->exp.t->u.array.size;
                op5 = initOp(TEMP);
                op6 = initOp(TEMP);
                Operand *_op5 = (Operand *)malloc(sizeof(Operand));
                Operand *_op6 = (Operand *)malloc(sizeof(Operand));
                _op5->kind = GETVAL;
                _op5->value = op5->value;
                _op6->kind = GETVAL;
                _op6->value = op6->value;
                insertIC(initIC(ASSIGN, op5, _op1, 0));
                insertIC(initIC(ASSIGN, op6, _op2, 0));
                insertIC(initIC(ASSIGN, count, zero, 0));
                insertIC(initIC(LAB, op3, 0, 0));
                insertIC(initIC(ASSIGN, _op5, _op6, 0));
                insertIC(initIC(ADD, op5, op5, op4));
                insertIC(initIC(ADD, op6, op6, op4));
                insertIC(initIC(ADD, count, count, one));
                ic1 = initIC(IFGO, count, size1, last);
                ic1->name = ">=";
                insertIC(ic1);
                ic2 = initIC(IFGO, count, size2, last);
                ic2->name = ">=";
                insertIC(ic2);
                insertIC(initIC(GOTO, op3, 0, 0));
                insertIC(initIC(LAB, last, 0, 0));
            }
            else
            {
                op2 = initOp(TEMP);
                translate(root->firstChild->nextSibling->nextSibling, op2, 0);
                translate(root->firstChild, op1, 0);
                insertIC(initIC(ASSIGN, op1, op2, 0));
            }
            //设置返回值
            if (arg1)
            {
                arg1->kind = op2->kind;
                arg1->value = op2->value;
            }
            break;
        case 2:
        case 3:
        case 4:
        case 11: // AND OR NOT RELOP
            op1 = initOp(LABEL);
            op2 = initOp(LABEL);
            insertIC(initIC(ASSIGN, arg1, zero, 0));
            Exp_Cond(root, op1, op2);
            insertIC(initIC(LAB, op1, 0, 0));
            insertIC(initIC(ASSIGN, arg1, one, 0));
            insertIC(initIC(LAB, op2, 0, 0));
            break;
        case 5:
        case 6:
        case 7:
        case 8: // Exp +-*/ Exp
            op1 = initOp(TEMP);
            op2 = initOp(TEMP);
            translate(root->firstChild, op1, 0);
            translate(root->firstChild->nextSibling->nextSibling, op2, 0);
            //! 下一行第一个参数，类型需要注意
            insertIC(initIC(root->expand - 4, arg1, op1, op2));
            break;
        case 9: // ( Exp )
            translate(root->firstChild->nextSibling, arg1, arg2);
            break;
        case 10: // - Exp
            op1 = initOp(TEMP);
            translate(root->firstChild->nextSibling, op1, 0);
            insertIC(initIC(SUB, arg1, zero, op1));
            break;
        case 12: // ID ( Args )
            //op1.next为第一个参数
            op1 = (Operand *)malloc(sizeof(Operand));
            translate(root->firstChild->nextSibling->nextSibling, op1, 0);
            if (strcmp(root->firstChild->c, "write") == 0)
            {
                insertIC(initIC(WRITE, op1->next, 0, 0));
                if (arg1)
                    insertIC(initIC(ASSIGN, arg1, zero, 0));
            }
            else
            {
                for (op2 = op1->next; op2; op2 = op2->next)
                    insertIC(initIC(ARG, op2, 0, 0));
                p = findSymbol(root->firstChild->c);
                insertIC(initIC(CALL, arg1, p->op, 0));
            }
            break;
        case 13: // ID ()
            if (strcmp(root->firstChild->c, "read") == 0)
                insertIC(initIC(READ, arg1, 0, 0));
            else
            {
                p = findSymbol(root->firstChild->c);
                insertIC(initIC(CALL, arg1, p->op, 0));
            }
            break;
        case 14: // E [ E ] 只会出现一维数组
            //检查多维数组
            if(root->firstChild->expand == 14){
                error();
            }
            //计算序号
            op1 = initOp(TEMP);
            translate(root->firstChild->nextSibling->nextSibling, op1, 0);
            //计算基地址
            op2 = initOp(GETADDR);
            translate(root->firstChild, op2, 0);
            //计算类型大小
            op3 = initOp(CONSTANT);
            op3->value = TypeSize(root->exp.t);
            //计算偏移
            op4 = initOp(TEMP);
            op5 = (Operand *)malloc(sizeof(Operand));
            op5->value = op4->value;
            op5->kind = TEMP;
            if (root->firstChild->expand != 16)
                op2->kind = TEMP;
            op6 = initOp(TEMP);
            insertIC(initIC(MUL, op6, op1, op3));
            insertIC(initIC(ADD, op5, op2, op6));
            if (!arg1)
                arg1 = initOp(TEMP);
            if (arg1->kind == TEMP) //读数据
            {
                op4->kind = GETVAL;
                insertIC(initIC(ASSIGN, arg1, op4, 0));
            }
            else // GETADDR
                arg1->value = op4->value;
            break;
        case 15:                    // E . ID
            //计算基地址
            op1 = initOp(GETADDR);
            translate(root->firstChild, op1, 0);
            //计算偏移
            op2 = initOp(CONSTANT);
            op2->value = 0;
            p = root->firstChild->exp.t->u.structure;
            while (p)
            {
                if (strcmp(p->name, root->firstChild->nextSibling->nextSibling->c) != 0)
                    op2->value += TypeSize(p->type);
                //! 数组需要统计全部大小
                else
                    break;
                p = p->next;
            }
            //计算偏移地址
            op3 = initOp(TEMP);
            op4 = (Operand *)malloc(sizeof(Operand));
            op4->kind = TEMP;
            op4->value = op3->value;
            if (root->firstChild->expand != 16)
                op1->kind = TEMP;
            insertIC(initIC(ADD, op4, op1, op2));
            if (!arg1)
                arg1 = initOp(TEMP);
            if (arg1->kind == TEMP) //读数据
            {
                op3->kind = GETVAL;
                insertIC(initIC(ASSIGN, arg1, op3, 0));
            }
            else // GETADDR
                arg1->value = op3->value;
            break;
        case 16: // ID 变量必定已经声明过，直接在符号表中搜索
            if (!arg1)
                break;
            p = findSymbol(root->firstChild->c);
            arg1->value = ((Operand *)(p->op))->value;
            if (p->type->kind == BASIC || p->param) //函数参数，不需要
                arg1->kind = VARIABLE;
            break;
        case 17: // INT
            if (!arg1)
                break;
            arg1->kind = CONSTANT;
            arg1->value = root->firstChild->i;
            break;
        default:
            break;
        }
        break;
    case Args:
        if (root->firstChild->exp.t->kind == STRUCTURE)
            op1 = initOp(GETADDR);
        else
            op1 = initOp(TEMP);
        translate(root->firstChild, op1, 0);
        if (root->firstChild->expand == 14)
            op1->kind = TEMP;
        op1->next = arg1->next;
        arg1->next = op1;
        if (root->expand == 1) // Exp , Args
            translate(root->firstChild->nextSibling->nextSibling, arg1, 0);
        break;
    default:
        break;
    }
}

void Exp_Cond(Node *root, Operand *tl, Operand *fl)
{
    Operand *op1, *op2;
    InterCode *ic1;
    switch (root->expand)
    {
    case 2: // Exp AND Exp
        op1 = initOp(LABEL);
        Exp_Cond(root->firstChild, op1, fl);
        insertIC(initIC(LAB, op1, 0, 0));
        Exp_Cond(root->firstChild->nextSibling->nextSibling, tl, fl);
        break;
    case 3: // Exp OR Exp
        op1 = initOp(LABEL);
        Exp_Cond(root->firstChild, tl, op1);
        insertIC(initIC(LAB, op1, 0, 0));
        Exp_Cond(root->firstChild->nextSibling->nextSibling, tl, fl);
        break;
    case 4: // Exp RELOP Exp
        op1 = initOp(TEMP);
        op2 = initOp(TEMP);
        translate(root->firstChild, op1, 0);
        translate(root->firstChild->nextSibling->nextSibling, op2, 0);
        ic1 = initIC(IFGO, op1, op2, tl);
        ic1->name = root->firstChild->nextSibling->c;
        insertIC(ic1);
        insertIC(initIC(GOTO, fl, 0, 0));
        break;
    case 11: // NOT Exp
        Exp_Cond(root->firstChild->nextSibling, fl, tl);
        break;
    default:
        op1 = initOp(TEMP);
        translate(root, op1, 0);
        ic1 = initIC(IFGO, op1, zero, tl);
        ic1->name = "!=";
        insertIC(ic1);
        insertIC(initIC(GOTO, fl, 0, 0));
        break;
    }
}

char *operand2string(Operand* op)
{
    int length = 1;
    int temp = op->value;
    while (temp)
    {
        temp = temp / 10;
        length++;
    }
    //+1是t/f/#,+1是'0',+1是可能取地址或取值
    char *result = (char *)malloc((length + 5) * sizeof(char));
    switch (op->kind)
    {
    case VARIABLE:
    case TEMP:
        sprintf(result, "t%d", op->value);
        break;
    case LABEL:
        sprintf(result, "l%d", op->value);
        break;
    case FUN:
        if(mainfun == op){
            sprintf(result, "main");
        }
        else{
            sprintf(result, "f%d", op->value);
        }
        break;
    case CONSTANT:
        sprintf(result, "#%d", op->value);
        break;
    case GETADDR:
        sprintf(result,"&t%d",op->value);
        break;
    case GETVAL:
        sprintf(result,"*t%d",op->value);
        break;
    default:
        assert(0);
        break;
    }
    return result;
}

bool opequal(Operand*op1,Operand*op2){
    if(op1->kind == op2->kind && op1->kind == VARIABLE && op1->value == op2->value){
        return true;
    }
    return false;
}
void skipic(){
    InterCode*now = IChead -> next;
    while (now)
    {
        switch (now->kind)
        {
        case ASSIGN:
            if(opequal(now->arg1,now->arg2)){
                now->skip = 1;
            }
            break;
        case ADD:
            if((opequal(now->arg1,now->arg2) && now->arg3->kind == CONSTANT && now->arg3->value == 0)||
                (opequal(now->arg1,now->arg3) && now->arg2->kind == CONSTANT && now->arg2->value == 0)
            ){
                now->skip = 1;
            }
            break;
        case MUL:
            if((opequal(now->arg1,now->arg2) && now->arg3->kind == CONSTANT && now->arg3->value == 1)||
                (opequal(now->arg2,now->arg3) && now->arg2->kind == CONSTANT && now->arg2->value == 1)
            ){
                now->skip = 1;
            }
            break;
        default:
            break;
        }
        now = now->next;
    }
    
}

void InterCodeoutput(char *filename)
{
    skipic();
    InterCode *now = IChead;
    now = now->next;//从链表头移动到第一个元素
    FILE *f = fopen(filename, "w");
    char *arg1 = 0, *arg2 = 0, *arg3 = 0;
    while (now)
    {
        if(now->skip == 1){
            now = now->next;
            continue;
        }
        arg1 = 0,arg2 = 0,arg3 = 0;
        arg1 = operand2string(now->arg1);
        switch (now->kind)
        {
        case ASSIGN:
            arg2 = operand2string(now->arg2);
            fprintf(f, "%s := %s", arg1, arg2);
            break;
        case ADD:
            arg2 = operand2string(now->arg2);
            arg3 = operand2string(now->arg3);
            fprintf(f, "%s := %s + %s", arg1, arg2, arg3);
            break;
        case SUB:
            arg2 = operand2string(now->arg2);
            arg3 = operand2string(now->arg3);
            fprintf(f, "%s := %s - %s", arg1, arg2, arg3);
            break;
        case MUL:
            arg2 = operand2string(now->arg2);
            arg3 = operand2string(now->arg3);
            fprintf(f, "%s := %s * %s", arg1, arg2, arg3);
            break;
        case DIVI:
            arg2 = operand2string(now->arg2);
            arg3 = operand2string(now->arg3);
            fprintf(f, "%s := %s / %s", arg1, arg2, arg3);
            break;
        case FUNC:
            fprintf(f,"FUNCTION %s :",arg1);
            break;
        case PARAM:
            fprintf(f,"PARAM %s",arg1);
            break;
        case DEC:
            fprintf(f, "DEC %s %d", arg1, now->arg1->size);
            break;
        case RET:
            fprintf(f,"RETURN %s",arg1);
            break;
        case LAB:
            fprintf(f,"LABEL %s :",arg1);
            break;
        case GOTO:
            fprintf(f,"GOTO %s",arg1);
            break;
        case CALL:
            arg2 = operand2string(now->arg2);
            fprintf(f, "%s := CALL %s", arg1, arg2);
            break;
        case ARG:
            fprintf(f,"ARG %s",arg1);
            break;
        case READ:
            fprintf(f,"READ %s",arg1);
            break;
        case WRITE:
            fprintf(f,"WRITE %s",arg1);
            break;
        case IFGO:
            arg2 = operand2string(now->arg2);
            arg3 = operand2string(now->arg3);
            fprintf(f,"IF %s %s %s GOTO %s",arg1,now->name,arg2,arg3);
            break;
        default:
            break;
        }
        if (now->next)
        {
            fprintf(f, "\n");
        }
        now = now->next;
        if (arg1)
        {
            free(arg1);
        }
        if (arg2)
        {
            free(arg2);
        }
        if (arg3)
        {
            free(arg3);
        }
    }
    fclose(f);
}

InterCode *initIC(int k, Operand *arg1, Operand *arg2, Operand *arg3)
{
    InterCode* result = (InterCode*)malloc(sizeof(InterCode));
    result->skip = 0;
    result->kind = k;
    result->arg1 = arg1;
    result->arg2 = arg2;
    result->arg3 = arg3;
    if(result->arg1 == 0 &&
        (k == CALL || k == ADD || k == ASSIGN || k == SUB || k == MUL || k == DIVI)
    ){
        result->arg1 = initOp(TEMP);
        if(k != CALL){
            result->skip = 1;
        }
    }
    return result;
}

void insertIC(InterCode *root)
{
    static InterCode *last = 0;
    if (!last)
    {
        last = IChead;
    }
    last->next = root;
    last = root;
}

Operand *initOp(int k)
{
    Operand* result = (Operand*)malloc(sizeof(Operand));
    result->kind = k;
    switch (k)
    {
    case VARIABLE:
    case TEMP:
        result->value = varCount++;
        break;
    case FUN:
        result->value = funCount++;
        break;
    case LABEL:
        result->value = labelCount++;
        break;
    default:
        break;
    }
    return result;
}