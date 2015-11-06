// tool.h, 159

#ifndef _TOOL_H
#define _TOOL_H

#include "type.h" // q_t needs be defined in code below

void MyBzero(char *, int);
int DeQ(q_t *);
void EnQ(int, q_t *);
void MsgEnQ(msg_t *p,mbox_t *q);
msg_t* MsgDeQ(mbox_t *p); 
void MyStrcpy(char *, char *);
int MyStrlen(char *str);
int MyStrcmp(char *str1, char *str2, int size);

#endif

