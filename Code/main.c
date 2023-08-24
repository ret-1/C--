#include <stdio.h>
#include "tree.h"
#include "ir.h"
#include "mips.h"
extern FILE *yyin;
extern Node *root;
extern int lexical_e;
extern int syntax_e;
int yyrestart(void *);
int yyparse();
void initCheck();
int main(int argc, char **argv)
{
    if (argc < 2)
        return 1;
    FILE *f = fopen(argv[1], "r");
    if (!f)
    {
        perror(argv[1]);
        return 1;
    }
    initHash();
    initCheck();
    yyrestart(f);
    yyparse();
    //if (!lexical_e && !syntax_e)
    //printt(root, 0);
    translate(root, 0, 0);
    //InterCodeoutput(argv[3]);
    mipsMain(argv[2]);

    return 0;
}