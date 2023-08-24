//#include "tree.h"
#include "symbol.h"
#include <string.h>
#include <stdlib.h>
unsigned int DJBHash(const char *str)    
{    
    unsigned int hash = 5381;    
     
    while (*str){    
        hash = ((hash << 5) + hash) + (*str++); 
    }    
    hash &= ~(1 << 31);
    return hash % tablesize;
}    

FieldList *insertSymbol(char *name, Type *type, bool access)
{
    unsigned int index = DJBHash(name);
    FieldList *target = symTable[index];
    if (target == NULL)
    {
        FieldList *result = (FieldList *)malloc(sizeof(FieldList));
        //strcpy(result->name, name);
        result->name = name;
        result->type = type;
        result->access = access;
        result->tail = NULL;
        symTable[index] = result;
        return result;
    }
    while (target->tail != NULL)
    {
        if (strcmp(name, target->name) == 0)
        {
            return NULL;
        }
        target = target->tail;
    }
    if (strcmp(name, target->name) == 0)
    {
        return NULL;
    }
    FieldList *result = (FieldList *)malloc(sizeof(FieldList));
    //strcpy(result->name, name);
    result->name = name;
    result->access = access;
    result->type = type;
    result->tail = NULL;
    target->tail = result;
    return result;
}

FieldList *findSymbol(const char *name)
{
    unsigned int index = DJBHash(name);
    FieldList *target = symTable[index];
    while (target)
    {
        if (strcmp(target->name, name) == 0)
        {
            return target;
        }
        target = target->tail;
    }
    return NULL;
}

FieldList *insertField(FieldList *target)
{
    int index = DJBHash(target->name);
    if (symTable[index] == NULL)
    {
        symTable[index] = target;
        return target;
    }
    FieldList *now = symTable[index];
    while (now->tail)
    {
        if (strcmp(now->name, target->name) == 0)
        {
            return NULL;
        }
        now = now->tail;
    }
    if (strcmp(now->name, target->name) == 0)
    {
        return NULL;
    }
    now->tail = target;
    return target;
}

bool StructIdentical(FieldList *a, FieldList *b)
{
    //assert(a->type->kind == STRUCTURE && b->type->kind == STRUCTURE);
    //a = a->next;
    //b = b->next;
    while (a && b)
    {
        if (!typeIdentical(a->type, b->type))
        {
            return false;
        }
        a = a->next;
        b = b->next;
    }
    if (a || b)
    {
        //两个结构体长度不同
        return false;
    }
    return true;
}

bool typeIdentical(Type *a, Type *b)
{
    //未定义两个函数的比较?
    if (a->kind != b->kind || a->kind == ERR || b->kind == ERR)
    {
        return false;
    }
    switch (a->kind)
    {
    case BASIC:
        return a->u.basic == b->u.basic;
        break;
    case ARRAY:
        return typeIdentical(a->u.array.elem, b->u.array.elem);
        break;
    case STRUCTURE:
        return StructIdentical(a->u.structure, b->u.structure);
        break;
    default:
        //assert(0);
        return false;
        break;
    }
}

void initHash()
{
    memset(symTable, 0, tablesize * sizeof(FieldList *));
}

int TypeSize(Type *t){
    assert(t);
    int result = 0;
    FieldList *temp = 0;
    switch (t->kind)
    {
    case BASIC:
        assert(t->u.basic == 0);
        return 4;
        break;
    case ARRAY:
        //只出现一维数组
        return TypeSize(t->u.array.elem) * t->u.array.size;
        break;
    case STRUCTURE:
        temp = t->u.structure;
        while (temp)
        {
            result += TypeSize(temp->type);
            temp = temp->next;
        }
        return result;
        break;
    default:
        assert(0);
        break;
    }
}