#include "tree.h"
#include <stdio.h>
#include <string.h>
#define child(i) vsp[i - n]
extern int inStruct;
extern FieldList *tType;
Type *errType, *temp;
FieldList *errField;
#define ERR_EXP(exp)   \
    (exp).t = errType; \
    (exp).val = LEFT;

void initCheck()
{
    errType = (Type *)malloc(sizeof(Type));
    errType->kind = ERR;
    errField = (FieldList *)malloc(sizeof(FieldList));
    errField->type = errType;
    temp = (Type *)malloc(sizeof(Type));
    FieldList *rf = (FieldList *)malloc(sizeof(FieldList));
    rf->access = true;
    rf->name = "read";
    rf->type = (Type *)malloc(sizeof(Type));
    rf->type->kind = FUNCTION;
    rf->type->u.function.params = 0;
    rf->type->u.function.rt = (Type *)malloc(sizeof(Type));
    rf->type->u.function.rt->kind = BASIC;
    rf->type->u.function.rt->u.basic = 0;
    insertField(rf);
    FieldList *wf = (FieldList *)malloc(sizeof(FieldList));
    wf->access = true;
    wf->name = "write";
    wf->type = (Type *)malloc(sizeof(Type));
    wf->type->kind = FUNCTION;
    wf->type->u.function.rt = (Type *)malloc(sizeof(Type));
    wf->type->u.function.rt->kind = BASIC;
    wf->type->u.function.rt->u.basic = 0;
    wf->type->u.function.params = (FieldList *)malloc(sizeof(FieldList));
    wf->type->u.function.params->type = wf->type->u.function.rt;
    insertField(wf);
}
void printarray(Type *type){
    switch (type->kind)
    {
    case BASIC:
        if(type->u.basic==0){
            printf("int");
        }
        else{
            printf("float");
        }
        break;
    case ARRAY:
        printarray(type->u.array.elem);
        printf("[]");
        break;
    case STRUCTURE:
        printf(" %s", type->name);
        break;
    default:
        break;
    }
}

void printArgs(FieldList *args)
{
    while (args)
    {
        switch (args->type->kind)
        {
        case BASIC:
            if(args->type->u.basic == 0){
                printf("int");
            }
            else{
                printf("float");
            }
            break;
        case STRUCTURE:
            printf("%s", args->type->name);
            break;
        case ARRAY:
            printarray(args->type);
            break;
        default:
            break;
        }
        if(args->next){
            printf(",");
        }
        args = args->next;
    }
    return;
}
void check(int idx, Node *root, Node **vsp, int n)
{
    FieldList *sym, *func, *p1;
    Type *t1, *t2;
    switch (idx)
    {
    //Exp -> INT|FLOAT
    //设置exp属性
    case 0:
        root->exp.t = (Type *)malloc(sizeof(Type));
        root->exp.t->kind = BASIC;
        if (child(1)->type == INT)
            root->exp.t->u.basic = 0;
        else
            root->exp.t->u.basic = 1;
        root->exp.val = RIGHT;
        break;
    //Exp -> ID
    //在符号表查找变量，找不到则设置为ERR
    case 1:
        sym = findSymbol(child(1)->c);
        if (!sym || !sym->access || sym->def)
        {
            printf("Error type 1 at Line %d: Undefined variable \"%s\".\n", child(1)->line, child(1)->c);
            ERR_EXP(root->exp)
            break;
        }
        root->exp.t = sym->type;
        root->exp.val = LEFT;
        if (sym->type->kind == FUNCTION)
            root->exp.val = RIGHT;
        break;
    //Exp -> Exp DOT ID
    //在结构体链表中顺序查找,直接在符号表查找似乎也可以？
    case 2:
        if (child(1)->exp.t->kind != STRUCTURE)
        {
            printf("Error type 13 at Line %d: Illegal use of \".\".\n", child(2)->line);
            ERR_EXP(root->exp)
            break;
        }
        sym = child(1)->exp.t->u.structure;
        while (sym)
        {
            if (strcmp(sym->name, child(3)->c) == 0)
            {
                root->exp.t = sym->type;
                root->exp.val = LEFT;
                break;
            }
            sym = sym->next;
        }
        if (!sym)
        {
            printf("Error type 14 at Line %d: Non-existent field \"%s\".\n", child(3)->line, child(3)->c);
            ERR_EXP(root->exp)
        }
        break;
    //Exp -> Exp [Exp]
    case 3:
        if (child(1)->exp.t->kind != ARRAY)
        {
            printf("Error type 10 at Line %d: \"%s\" is not an array.\n", child(2)->line, child(1)->c);
            ERR_EXP(root->exp)
            break;
        }
        if (!(child(3)->exp.t->kind == BASIC && child(3)->exp.t->u.basic == 0))
        {
            printf("Error type 12 at Line %d: Expression in the brackets must be an integer.\n", child(3)->line);
            ERR_EXP(root->exp)
            break;
        }
        root->exp.t = child(1)->exp.t->u.array.elem;
        root->exp.val = LEFT;
        break;
    //Exp -> ID (Args)|ID ()
    //将exp类型设置为函数返回值类型(右值)，出错则设置为ERR
    case 4:
        func = findSymbol(child(1)->c);
        if (!func)
        {
            printf("Error type 2 at Line %d: Undefined function \"%s\".\n", child(1)->line, child(1)->c);
            ERR_EXP(root->exp)
            break;
        }
        if (func->type->kind != FUNCTION)
        {
            printf("Error type 11 at Line %d: \"%s\" is not a function.\n", child(1)->line, child(1)->c);
            ERR_EXP(root->exp)
            break;
        }
        p1 = func->type->u.function.params;
        if (n == 3) //Exp -> ID()
        {
            if (p1)
            {
                printf("Error type 9 at Line %d: Function \"%s", child(1)->line, child(1)->c);
                printArgs(p1);
                printf("\" is not applicable for arguments \"()\".\n");
                //ERR_EXP(root->exp)
            }
        }
        else //Exp -> ID(Args)
        {
            FieldList *p2 = child(3)->info;
            if (!StructIdentical(p1, p2))
            {
                printf("Error type 9 at Line %d: Function \"%s(", child(1)->line, child(1)->c);
                printArgs(p1);
                printf(")\" is not applicable for arguments \"(");
                printArgs(p2);
                printf(")\".\n");
                //ERR_EXP(root->exp)
            }
        }
        root->exp.t = func->type->u.function.rt;
        root->exp.val = RIGHT;
        break;
    //Exp -> NOT Exp
    case 5:
        if (!(child(2)->exp.t->kind == BASIC && child(2)->exp.t->u.basic == 0))
        {
            printf("Error type 7 at Line %d: Type mismatched for operands.\n", child(1)->line);
            ERR_EXP(root->exp)
            break;
        }
        root->exp.t = child(2)->exp.t;
        root->exp.val = RIGHT;
        break;
    //Exp -> - Exp
    case 6:
        if (child(2)->exp.t->kind != BASIC)
        {
            printf("Error type 7 at Line %d: Type mismatched for operands.\n", child(1)->line);
            ERR_EXP(root->exp)
            break;
        }
        root->exp.t = child(2)->exp.t;
        root->exp.val = RIGHT;
        break;
    //Exp -> (Exp)
    case 7:
        root->exp.t = child(2)->exp.t;
        root->exp.val = child(2)->exp.val;
        break;
    //Exp -> Exp +-*/RELOP Exp
    case 8:
        t1 = child(1)->exp.t;
        t2 = child(3)->exp.t;
        //同为int或者同为float才能进行算数运算
        if (!(t1->kind == BASIC && t2->kind == BASIC && t1->u.basic == t2->u.basic))
        {
            printf("Error type 7 at Line %d: Type mismatched for operands.\n", child(2)->line);
            ERR_EXP(root->exp)
            break;
        }
        //比较运算，设置为int
        if (child(2)->type == RELOP)
        {
            root->exp.t = (Type *)malloc(sizeof(Type));
            root->exp.t->kind = BASIC;
            root->exp.t->u.basic = 0;
        }
        else
            root->exp.t = t1;
        root->exp.val = RIGHT;
        break;
    //Exp -> Exp AND|OR Exp 
    case 9:
        t1 = child(1)->exp.t;
        t2 = child(3)->exp.t;
        //只有int能进行逻辑运算
        if (!(t1->kind == BASIC && t2->kind == BASIC && t1->u.basic == 0 && t2->u.basic == 0))
        {
            printf("Error type 7 at Line %d: Type mismatched for operands.\n", child(2)->line);
            ERR_EXP(root->exp)
            break;
        }
        root->exp.t = t1;
        root->exp.val = RIGHT;
        break;
    //Exp -> Exp = Exp
    case 10:
        t1 = child(1)->exp.t;
        t2 = child(3)->exp.t;
        if (child(1)->exp.val == RIGHT)
        {
            printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n", child(1)->line);
            ERR_EXP(root->exp)
            break;
        }

        if (!typeIdentical(t1, t2))
        {
            printf("Error type 5 at Line %d: Type mismatched for assignment.\n", child(3)->line);
            ERR_EXP(root->exp)
            break;
        }
        //赋值语句返回左边的值,且为左值
        root->exp.t = t1;
        root->exp.val = LEFT;
        break;
    //Args -> Exp | Exp,Args
    //通过next连接成链表，info为head
    case 11:
        root->info = (FieldList *)malloc(sizeof(FieldList));
        root->info->type = child(1)->exp.t;
        root->info->next = n == 1 ? NULL : child(3)->info;
        break;
    //VarDec -> ID
    case 12:
        sym = findSymbol(child(1)->c);
        if (sym)
        {
            if (sym->access)
                printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", child(1)->line, child(1)->c);
            else
                printf("Error type 15 at Line %d: Redefined field \"%s\".\n", child(1)->line, child(1)->c);
            root->info = NULL;
            root->last = NULL;
            break;
        }
        root->info = (FieldList *)malloc(sizeof(FieldList));
        root->info->name = child(1)->c;
        root->info->type = (Type *)malloc(sizeof(Type));
        //last表示变量最后的type
        root->last = root->info->type;
        root->info->access = !inStruct;
        root->info->def = false;
        //在最初就进行插入，后续通过指针完成类型的确定
        insertField(root->info);
        break;
    //VarDec -> VarDec [INT]
    case 13:
        root->info = child(1)->info;
        if (!root->info)
            break;
        child(1)->last->kind = ARRAY;
        child(1)->last->u.array.size = child(3)->i;
        child(1)->last->u.array.elem = (Type *)malloc(sizeof(Type));
        root->last = child(1)->last->u.array.elem;
        break;
    //ParamDec -> Specifier VarDec
    case 14:
        if (child(2)->info)
            *(child(2)->last) = *(child(1)->info->type); //确定最终类型
        root->info = child(2)->info;
        //将栈顶元素弹出
        if (tType)
            tType = tType->tail;
        break;
    //VarList -> ParamDec , VarList | ParamDec
    //组织成链表
    case 15:
        root->info = child(1)->info;
        if (root->info)
            root->info->next = n == 1 ? NULL : child(3)->info;
        break;
    //FunDec -> ID ( VarList ) | ID ()
    case 16:
        if (findSymbol(child(1)->c))
        {
            printf("Error type 4 at Line %d: Redefined function \"%s\".\n", child(1)->line, child(1)->c);
            root->info = NULL;
            break;
        }
        root->info = (FieldList *)malloc(sizeof(FieldList));
        root->info->name = child(1)->c;
        root->info->type = (Type *)malloc(sizeof(Type));
        root->info->type->kind = FUNCTION;
        root->info->type->u.function.params = n == 3 ? NULL : child(3)->info;
        break;
    //Dec -> VarDec | VarDec ASSIGNOP Exp
    case 17:
        root->info = child(1)->info;
        if (!root->info)
            break;
        if (child(1)->last)
            *(child(1)->last) = *(tType->type); //使用栈顶类型赋值
        if (n == 3 && inStruct)
        {
            printf("Error type 15 at Line %d: Illegal use of assignment.\n", child(2)->line);
            break;
        }
        if (n == 3 && !typeIdentical(child(3)->exp.t, tType->type))
        {
            printf("Error type 5 at Line %d: Type mismatched for assignment.\n", child(3)->line);
            break;
        }
        break;
    //DecList -> Dec | Dec COMMA DecList
    case 18:
        if (child(1)->info)
        {
            root->info = child(1)->info;
            root->info->next = n == 1 ? NULL : child(3)->info;
        }
        else if (n == 1)
            root->info = NULL;
        else
            root->info = child(3)->info;
        break;
    //Def -> Specifier DecList SEMI
    case 19:
        root->info = child(2)->info;
        if (tType)
            tType = tType->tail;
        break;
    //DefList -> Def DefList
    case 20:
        root->info = child(1)->info;
        //Def为null，直接将DefList赋给上级
        if (!root->info)
        {
            root->info = child(2) ? child(2)->info : NULL;
            break;
        }
        //Def可能有多个变量，因此遍历到末尾进行链接
        sym = root->info;
        while (sym->next)
        //TODO:
            sym = sym->next;
        sym->next = child(2) ? child(2)->info : NULL;
        break;
    // IF/WHILE ( Exp )
    case 21:
        if (!(child(3)->exp.t->kind == BASIC && child(3)->exp.t->u.basic == 0))
            printf("Error type 7 at Line %d: Type mismatched.\n", child(3)->line);
        break;
    //RETURN
    case 22:
        if (!typeIdentical(child(2)->exp.t, tType->type))
            printf("Error type 8 at Line %d: Type mismatched for return.\n", child(2)->line);
        break;
    //StructSpecifier -> STRUCT Tag
    case 23:
        sym = findSymbol(child(2)->firstChild->c);
        if (sym)
            root->info = sym;
        else
        {
            printf("Error type 17 at Line %d: Undefined structure \"%s\".\n", child(2)->line, child(2)->firstChild->c);
            root->info = errField;
        }
        break;
    //StructSpecifier -> STRUCT OptTag { // DefList // }
    case 24:
        t1 = (Type *)malloc(sizeof(Type));
        t1->kind = STRUCTURE;
        t1->u.structure = child(5) ? child(5)->info : NULL;
        if (child(2) && child(2)->firstChild) //OptTag -> ID
        {
            if (findSymbol(child(2)->firstChild->c))
            {
                printf("Error type 16 at Line %d: Duplicated name \"%s\".\n", child(2)->line, child(2)->firstChild->c);
                root->info = (FieldList *)malloc(sizeof(FieldList));
                root->info->name = child(2)->firstChild->c;
                root->info->access = !inStruct;
                root->info->type = t1;
                root->info->type->name = child(2)->firstChild->c;
                root->info->def = true;
                break;
            }
            root->info = insertSymbol(child(2)->firstChild->c, t1, !inStruct);
            root->info->def = true;
        }
        else
        {
            //root->info = insertSymbol(NULL, t1, !inStruct);
            root->info = (FieldList *)malloc(sizeof(FieldList));
            root->info->name = "";
            root->info->access = !inStruct;
            root->info->type = t1;
            root->info->type->name = "";
        }
        root->info->type->name = root->info->name;
        break;
    //Specifier -> StructSpecifier
    case 25:
        root->info = child(1)->info;
        //入栈
        sym = (FieldList *)malloc(sizeof(FieldList));
        sym->tail = tType;
        tType = sym;
        //sym->type = root->info ? root->info->type : NULL;
        sym->type = root->info->type;
        break;
    //Specifier -> TYPE
    case 26:
        root->info = (FieldList *)malloc(sizeof(FieldList));
        root->info->type = (Type *)malloc(sizeof(Type));
        root->info->type->kind = BASIC;
        if (child(1)->i == INT)
            root->info->type->u.basic = 0;
        else
            root->info->type->u.basic = 1;
        sym = (FieldList *)malloc(sizeof(FieldList));
        sym->tail = tType;
        tType = sym;
        sym->type = root->info->type;
        break;
    //ExtDef -> Specifier FunDec ///// CompSt
    case 27:
        p1 = child(2)->info;
        //if (tType)
            //tType = tType->tail;
        if (!p1)
            break;
        p1->type->u.function.rt = child(1)->info->type;
        p1->access = true;
        insertField(p1);
        break;
    //ExtDef -> Specifier SEMI
    case 28:
        if (tType)
            tType = tType->tail;
        break;
    //ExtDecList -> VarDec
    case 29:
        root->info = child(1)->info;
        if (root->info)
        {
            root->info->next = NULL;
            if (child(1)->last)
                *(child(1)->last) = *(tType->type); //使用栈顶类型赋值
        }
        break;
    //ExtDecList -> VarDec COMMA ExtDecList
    case 30:
        if (child(1)->info)
        {
            root->info = child(1)->info;
            root->info->next = child(3)->info;
            if (child(1)->last)
                *(child(1)->last) = *(tType->type); //使用栈顶类型赋值
        }
        else
            root->info = child(3)->info;
        break;
    default:
        break;
    }
}