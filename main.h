// main.h, 159

#ifndef _MAIN_H_
#define _MAIN_H_
#include "type.h"

int main();
void SetData();
void Scheduler();
void SetControl();
void SetEntry(int entry_num, func_ptr_t func_ptr);
void Kernel(trapframe_t *p);


#endif
