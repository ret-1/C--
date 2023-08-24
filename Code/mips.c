#include "mips.h"
#include <stdio.h>
#include <string.h>

FILE *mips;
int *offsets;
const char begin[] = ".data\n\
_prompt: .asciiz \"Enter an integer:\"\n\
_ret: .asciiz \"\\n\"\n\
.globl main\n\
.text\n\
read:\n\
  li $v0, 4\n\
  la $a0, _prompt\n\
  syscall\n\
  li $v0, 5\n\
  syscall\n\
  jr $ra\n\
\n\
write:\n\
  li $v0, 1\n\
  syscall\n\
  li $v0, 4\n\
  la $a0, _ret\n\
  syscall\n\
  move $v0, $0\n\
  jr $ra";

int argCount = 0;
int offset = 0;

void mipsMain(const char *filename)
{
    offsets = (int *)malloc(varCount * sizeof(int));
    for (int i = 0; i < varCount; i++)
    {
        offsets[i] = -1;
    }
    mips = fopen(filename, "w");
    fprintf(mips, begin);
    InterCode *p = IChead->next;
    while (p)
        p = selectInst(p);
    fclose(mips);
}

InterCode *selectInst(InterCode *p)
{
    InterCode *ret;
    //TODO 检查是否可以合并中间代码，并对op123赋值，选择合适的指令
    ret = p->next;
    genMips(p);
    return ret;
}

void genMips(InterCode *p)
{
    int x, y, z; //寄存器
    x = getReg(p->arg1, p);
    y = getReg(p->arg2, p);
    z = getReg(p->arg3, p);
    //似乎不会出现&操作
    switch (p->kind)
    {
    case ASSIGN:
        switch (p->arg1->kind)
        {
        case VARIABLE:
        case TEMP:
            switch (p->arg2->kind)
            {
            case VARIABLE:
            case TEMP:
                // x := y
                fprintf(mips, "  move $%d, $%d\n", x, y);
                break;
            case GETVAL:
                // x := *y
                fprintf(mips, "  lw $%d, 0($%d)\n", x, y);
                break;
            case CONSTANT:
                // x := #k
                fprintf(mips, "  li $%d, %d\n", x, p->arg2->value);
                break;
            default:
                assert(0);
            }
            break;
        case GETVAL:
            switch (p->arg2->kind)
            {
            case VARIABLE:
            case TEMP:
                // *x := y
                fprintf(mips, "  sw $%d, 0($%d)\n", y, x);
                break;
            //! 以下两个操作先将第二个操作数载入一个寄存器
            case GETVAL:
                // *x := *y
                fprintf(mips, "  lw $%d, 0($%d)\n", z, y);
                fprintf(mips, "  sw $%d, 0($%d)\n", z, x);
                break;
            case CONSTANT:
                // *x := #k
                fprintf(mips, "  li $%d, %d\n", y, p->arg2->value);
                fprintf(mips, "  sw $%d, 0($%d)\n", y, x);
                break;
            default:
                assert(0);
            }
            break;
        default:
            assert(0);
        }
        break;
    case ADD:
        fprintf(mips, "  add $%d, $%d, $%d\n", x, y, z);
        break;
    case SUB:
        fprintf(mips, "  sub $%d, $%d, $%d\n", x, y, z);
        break;
    case MUL:
        fprintf(mips, "  mul $%d, $%d, $%d\n", x, y, z);
        break;
    case DIVI:
        fprintf(mips, "  div $%d, $%d\n", y, z);
        fprintf(mips, "  mflo $%d\n", x);
        break;
    case LAB:
        fprintf(mips, "%s:\n", operand2string(p->arg1));
        break;
    case GOTO:
        fprintf(mips, "  j %s\n", operand2string(p->arg1));
        break;
    case FUNC:
        fprintf(mips, "\n%s:\n", operand2string(p->arg1));
        // push $fp
        fprintf(mips, "  addi $sp, $sp, -4\n");
        fprintf(mips, "  sw $fp, 0($sp)\n");
        fprintf(mips, "  move $fp, $sp\n");
        //fprintf(mips, "  subu $sp, $sp, %d\n", 64);
        offset = 0;
        argCount = 0;
        break;
    case PARAM:
        //设置参数偏移
        if (argCount < 4)
        {
            offset += 4;
            fprintf(mips, "  addi $sp, $sp, -4\n");
            fprintf(mips, "  sw $a%d, 0($sp)\n", argCount);
            setoff(p->arg1->value, -offset);
        }
        else
        {
            setoff(p->arg1->value, 4 * (argCount - 2));
        }
        if (p->next->kind == PARAM)
            argCount++;
        else
            argCount = 0;
        break;
    case DEC:
        offset += p->arg1->size;
        fprintf(mips, "  subu $sp, $sp, %d\n", p->arg1->size);
        setoff(p->arg1->value, -offset);
        break;
    case ARG:
        switch (p->arg1->kind)
        {
        case VARIABLE:
        case TEMP:
            if (argCount < 4)
                fprintf(mips, "  move $a%d, $%d\n", argCount, x);
            else
            { // push arg
                fprintf(mips, "  addi $sp, $sp, -4\n");
                fprintf(mips, "  sw $%d, 0($sp)\n", x);
            }
            break;
        case GETVAL:
            // 使用$v1
            fprintf(mips, "  move $v1, $%d\n", x);
            if (argCount < 4)
                fprintf(mips, "  move $a%d, $v1\n", argCount);
            else
            { // push arg
                fprintf(mips, "  addi $sp, $sp, -4\n");
                fprintf(mips, "  sw $v1, 0($sp)\n");
            }
            break;
        case CONSTANT:
            if (argCount < 4)
                fprintf(mips, "  li $a%d, %d\n", argCount, p->arg1->value);
            else
            { // push arg
                fprintf(mips, "  li $v1, %d\n", p->arg1->value);
                fprintf(mips, "  addi $sp, $sp, -4\n");
                fprintf(mips, "  sw $v1, 0($sp)\n");
            }
            break;
        default:
            assert(0);
        }
        argCount++;
        break;
    case CALL:
        // push $ra 保存返回地址
        fprintf(mips, "  addi $sp, $sp, -4\n");
        fprintf(mips, "  sw $ra, 0($sp)\n");
        fprintf(mips, "  jal %s\n", operand2string(p->arg2));
        fprintf(mips, "  move $%d, $v0\n", x);
        //TODO 恢复寄存器?
        // pop $ra 恢复返回地址
        fprintf(mips, "  lw $ra, 0($sp)\n");
        fprintf(mips, "  addi $sp, $sp, 4\n");
        if (argCount > 4) //恢复offset
            fprintf(mips, "  addi $sp, $sp, %d\n", 4 * (argCount - 4));
        argCount = 0;
        break;
    case READ:
        // push $ra 保存返回地址
        fprintf(mips, "  addi $sp, $sp, -4\n");
        fprintf(mips, "  sw $ra, 0($sp)\n");
        // 跳转到read
        fprintf(mips, "  jal read\n");
        fprintf(mips, "  move $%d, $v0\n", x);
        //TODO 恢复寄存器?
        // pop $ra 恢复返回地址
        fprintf(mips, "  lw $ra, 0($sp)\n");
        fprintf(mips, "  addi $sp, $sp, 4\n");
        break;
    case WRITE:
        fprintf(mips, "  move $a0, $%d\n", x);
        // push $ra 保存返回地址
        fprintf(mips, "  addi $sp, $sp, -4\n");
        fprintf(mips, "  sw $ra, 0($sp)\n");
        // 跳转到read
        fprintf(mips, "  jal write\n");
        // 返回0，可有可无
        fprintf(mips, "  move $%d, $v0\n", x);
        //TODO 恢复寄存器?
        // pop $ra 恢复返回地址
        fprintf(mips, "  lw $ra, 0($sp)\n");
        fprintf(mips, "  addi $sp, $sp, 4\n");
        argCount = 0;
        break;
    case RET:
        fprintf(mips, "  move $v0, $%d\n", x);
        fprintf(mips, "  move $sp, $fp\n");
        // pop $fp
        fprintf(mips, "  lw $fp, 0($sp)\n");
        fprintf(mips, "  addi $sp, $sp, 4\n");
        fprintf(mips, "  jr $ra\n");
        break;
    case IFGO:
        if (strcmp(p->name, "==") == 0)
            fprintf(mips, "  beq $%d, $%d, %s\n", x, y, operand2string(p->arg3));
        else if (strcmp(p->name, "!=") == 0)
            fprintf(mips, "  bne $%d, $%d, %s\n", x, y, operand2string(p->arg3));
        else if (strcmp(p->name, ">") == 0)
            fprintf(mips, "  bgt $%d, $%d, %s\n", x, y, operand2string(p->arg3));
        else if (strcmp(p->name, "<") == 0)
            fprintf(mips, "  blt $%d, $%d, %s\n", x, y, operand2string(p->arg3));
        else if (strcmp(p->name, ">=") == 0)
            fprintf(mips, "  bge $%d, $%d, %s\n", x, y, operand2string(p->arg3));
        else if (strcmp(p->name, "<=") == 0)
            fprintf(mips, "  ble $%d, $%d, %s\n", x, y, operand2string(p->arg3));
        else
            assert(0);
        break;
    default:
        break;
    }
    update(p, x, y, z, 0);
}

int getoff(int value)
{
    // if(offsets[value] == -1){
    //     assert(0);
    // }
    return offsets[value];
}

void setoff(int value, int off)
{
    offsets[value] = off;
}

void update(InterCode *ic, int x, int y, int z, int code)
{
    //需要保存x的值
    if (x != -1 &&
        ic->arg1->kind != GETVAL &&
        (ic->kind == ASSIGN ||
         ic->kind == ADD ||
         ic->kind == SUB ||
         ic->kind == CALL ||
         ic->kind == MUL ||
         ic->kind == DIVI ||
         ic->kind == READ))
    {
        if (getoff(ic->arg1->value) == -1)
        {
            offset += 4;
            fprintf(mips, "  addi $sp, $sp, -4\n");
            setoff(ic->arg1->value, -offset);
        }
        fprintf(mips, "  sw $%d, %d($fp)\n", x, getoff(ic->arg1->value));
    }
    // if(x != -1 && ic->arg1->kind != CONSTANT){
    //     if(getoff(ic->arg1->value)==-1){
    //         offset += 4;
    //         fprintf(mips, "  addi $sp, $sp, -4\n");
    //         setoff(ic->arg1->value,-offset);
    //     }
    //     fprintf(mips, "  sw $%d, %d($fp)\n",x,getoff(ic->arg1->value));
    // }
    // if(y != -1 && ic->arg2->kind != CONSTANT){
    //     if(getoff(ic->arg2->value)==-1){
    //         offset += 4;
    //         fprintf(mips, "  addi $sp, $sp, -4\n");
    //         setoff(ic->arg2->value,-offset);
    //     }
    //     fprintf(mips, "  sw $%d, %d($fp)\n",y,getoff(ic->arg2->value));
    // }
    // if(z != -1 && ic->arg3->kind != CONSTANT){
    //     if(getoff(ic->arg3->value)==-1){
    //         offset += 4;
    //         fprintf(mips, "  addi $sp, $sp, -4\n");
    //         setoff(ic->arg3->value,-offset);
    //     }
    //     fprintf(mips, "  sw $%d, %d($fp)\n",z,getoff(ic->arg3->value));
    // }
}

int getReg(Operand *op, InterCode *ic)
{
    if (ic->kind == ASSIGN && ic->arg1->kind == GETVAL && ic->arg2->kind == GETVAL && op == ic->arg3)
    {
        return 4;
    }
    if (op == NULL || op->kind == LABEL || op->kind == FUN || ic->kind == PARAM || ic->kind == DEC)
    {
        return -1;
    }
    //乘除,IFGO遇到常数
    if (op->kind == CONSTANT)
    {
        if (ic->kind == ARG)
        {
            return -1;
        }
        if (ic->kind == MUL || ic->kind == DIVI || ic->kind == IFGO || ic->kind == RET || ic->kind == ADD || ic->kind == SUB)
        {
            if (op == ic->arg2)
            {
                fprintf(mips, "  li $3, %d\n", op->value);
                return 3;
            }
            else if (op == ic->arg3)
            {
                fprintf(mips, "  li $4, %d\n", op->value);
                return 4;
            }
            else
            {
                fprintf(mips, "  li $2, %d\n", op->value);
                return 2;
            }
        }
        else
        {
            if (op == ic->arg2)
            {
                return 3;
            }
            else if (op == ic->arg3)
            {
                return 4;
            }
        }
    }
    //需要改值
    if (op == ic->arg1 &&
        op->kind != GETVAL &&
        (ic->kind == ASSIGN ||
         ic->kind == ADD ||
         ic->kind == SUB ||
         ic->kind == CALL ||
         ic->kind == MUL ||
         ic->kind == DIVI ||
         ic->kind == READ))
    {
        return 2;
    }
    //只用取值
    if (op == ic->arg1)
    {
        fprintf(mips, "  lw $2, %d($fp)\n", getoff(op->value));
        return 2;
    }
    else if (op == ic->arg2)
    {
        if (op->kind == GETADDR)
        {
            //fprintf(mips, "  lw $3, 0($fp)\n");
            fprintf(mips, "  li $4, %d\n", getoff(op->value));
            fprintf(mips, "  add $3, $fp, $4\n");
        }
        else{
            fprintf(mips, "  lw $3, %d($fp)\n", getoff(op->value));
        }
        return 3;
    }
    else
    {
        fprintf(mips, "  lw $4, %d($fp)\n", getoff(op->value));
        return 4;
    }
}