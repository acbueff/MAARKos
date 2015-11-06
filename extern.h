// extern.h, 159

#ifndef _EXTERN_H_
#define _EXTERN_H_

#include "type.h"                        // q_t, pcb_t, MAX_PROC, STACK_SIZE


extern int run_pid;                      // ID of running process, -1 means not set
extern int system_time;

extern int system_print_semaphore;
extern semaphore_t semaphore[Q_SIZE];
extern q_t proc_q, unused_q, sleep_q, semaphore_q;             
extern mbox_t mbox[MAX_PROC];
extern pcb_t pcb[MAX_PROC];              // process table
extern char stack[MAX_PROC][STACK_SIZE]; // runtime stacks of processes
extern interface_t proc_interface;
//testing



#endif
