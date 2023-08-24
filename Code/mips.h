#ifndef __MIPS_H__
#define __MIPS_H__
#include "ir.h"
#define regsize 24//2-25
typedef struct Descriptor Descriptor;
typedef struct dc dc;
struct dc
{
    int value;//变量的序号,-1表示空
    int index;//地址描述符中的寄存器序号，-1表示内存地址
    dc* next;//地址描述符接下一个变量
    dc* nextaddr;//下一个地址
};


struct Descriptor
{
    dc reg[regsize];
    dc* addr;
    //Descriptor*next;
};

// 输出mips代码到文件
void mipsMain(const char *filename);

// 指令选择，生成mips代码，返回下一条中间代码
InterCode *selectInst(InterCode *p);

// 生成mips代码并输出
void genMips(InterCode* p);

//TODO 分配寄存器,设计数据结构
int getReg(Operand *op,InterCode*ic); //label,constant,null,func返回-1

//取出偏移量
int getoff(int value);//返回1表示没有

void update(InterCode*ic,int x,int y,int z,int code);

//设置偏移量
void setoff(int value,int off);
#endif