#include<stdlib.h>
#include<stdbool.h>
#include<assert.h>
#ifndef __SYMBOL_H__
#define __SYMBOL_H__
#define tablesize 666
typedef struct Type_ Type;
typedef struct FieldList_ FieldList;
struct FieldList_
{
    bool access;    //false表示不可从外界访问
    bool def;
    char *name;      // 域的名字/符号名
    Type* type;      // 域的类型/符号类型
    FieldList* tail; // 相同hash值的下一个域/符号
    FieldList* next; //结构体或函数参数列表的下一个符号
    void *op;
    bool param;
};

struct Type_
{
    enum
    {
        BASIC,
        ARRAY,
        STRUCTURE,
        FUNCTION,
        ERR
    } kind;
    union
    {
        // 基本类型 0 for int
        int basic;
        // 数组类型信息包括元素类型与数组大小构成
        struct
        {
            Type* elem;
            int size;
        } array;
        // 结构体类型信息是一个链表
        FieldList* structure;
        // 函数类型信息包括返回类型、函数名、参数列表
        struct
        {
            Type* rt;
            FieldList* params;
        } function;
    } u;
    char *name; //记录结构体的名字
};

FieldList* symTable[tablesize];

//hash函数
unsigned int DJBHash(const char *str);

void initHash();

// 返回类型大小
int TypeSize(Type *t);

//插入符号,返回NULL表示插入失败
FieldList *insertSymbol(char *name, Type *type, bool access);

FieldList* insertField(FieldList* target);

//查找
FieldList* findSymbol(const char *name);

bool StructIdentical(FieldList *a, FieldList *b);
bool typeIdentical(Type *a, Type *b);
#endif