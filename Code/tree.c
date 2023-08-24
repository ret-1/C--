#include "tree.h"
#include <string.h>
#include <stdio.h>
//类型、行号、yytext
Node *init(int type, int line, const char *text){
#ifdef DEBUG
    if(type==0)
        printf("type = NTERMINAL\n");
    else
        printf("type = %s\n",typename[type-INT]);
    printf("line = %d\n",line);
    printf("text = %s\n\n",text);
    //printf("%s\n",typename[0]);
#endif
    Node* result = (Node*)malloc(sizeof(Node));
    result->line = line;
    result->type = type;
    result->firstChild = NULL;
    result->nextSibling = NULL;
    switch(type){
        case RELOP:
            if( strcmp(text,"==")!=0 &&
                strcmp(text,">=")!=0 &&
                strcmp(text,"<=")!=0 &&
                strcmp(text,">")!=0 &&
                strcmp(text,"<")!=0 &&
                strcmp(text,"!=")!=0
            ){
                assert(0);
            }
            strcpy(result->c,text);
            break;
        case INT:
            if(strlen(text)<2)
                sscanf(text,"%u",&(result->i));
            else if(text[1]=='x')
                sscanf(text, "%x", &(result->i));
            else if(text[1]=='X')
                sscanf(text, "%X", &(result->i));
            else if(text[0]=='0')
                sscanf(text, "%o", &(result->i));
            else
                sscanf(text,"%u",&(result->i));
            break;
        case FLOAT:
            result->f = atof(text);
            break;
        case ID:
        case NTERMINAL:
            strcpy(result->c,text);
            break;
        case TYPE:
            if(strcmp(text,"int") == 0)
                result->i = INT;
            else if(strcmp(text,"float") == 0)
                result->i = FLOAT;
            else
                assert(0);
            break;
        default:break;
    }
    return result;
}
void insert(Node *f, Node *c){
    assert(f != NULL);
    if(!c) return;
    if(f->firstChild == NULL){
        f->firstChild = c;
        return;
    }
    Node *now = f->firstChild;
    while(now->nextSibling != NULL)
        now = now->nextSibling;
    now->nextSibling = c;
}

void printt(Node *r,const int indentcnt){
#ifdef lab1
    if(!r)
        return;
    for(int i = 0; i < indentcnt; i++)
        printf(" ");
    switch(r->type){
        case NTERMINAL:
            printf("%s (%d)\n",r->c,r->line);
            break;
        case ID:
            printf("ID: %s\n",r->c);
            break;
        case TYPE:
            if(r->i == INT)
                printf("TYPE: int\n");
            else if(r->i == FLOAT)
                printf("TYPE: float\n");
            else
                assert(0);
            break;
        case INT:
            printf("INT: %u\n",r->i);
            break;
        case FLOAT:
            printf("FLOAT: %f\n",r->f);
            break;
        default:
            printf("%s\n",typename[(r->type)- INT]);
            break;
        }
    printt(r->firstChild,indentcnt + 2);
    printt(r->nextSibling,indentcnt);
#endif
}