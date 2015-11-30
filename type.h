// type.h, 159

#ifndef _TYPE_H_
#define _TYPE_H_

#include "trapframe.h"

#define T_SLICE 10          // max timer ticks in 1 run
#define MAX_PROC 20          // max number of processes
#define Q_SIZE 20            // queuing capacity
#define STACK_SIZE 8196      // process runtime stack in bytes

// this is the same as constants defines: UNUSED=0, READY=1, etc.
typedef enum {UNUSED, READY, RUN, SLEEP, WAIT, ZOMBIE, WAIT4CHILD} state_t;

typedef struct {             // PCB describes proc image
   state_t state;            // state of process
   int time;                 // run time since loaded
   int total_time;    // total run time since created
   int wake_time;
   trapframe_t *trapframe_p; // points to trapframe of process
   int ppid;
 } pcb_t;

typedef struct {             // proc queue type
   int q[Q_SIZE];            // indices into q[] array to place or get element
   int head, tail, size;     // where head and tail are, and current size
} q_t;

typedef struct {
	q_t wait_q;
	int pass_count;
}semaphore_t;

typedef struct {
      int sender;         // sender
      int time_stamp;    // time sent
      char data[101];           // just this for now
	  int code;
	  int number[3];
}msg_t;

typedef struct {
      msg_t msg[Q_SIZE];
      int head, tail, size;
      q_t wait_q;
}mbox_t;

typedef struct {    // Phase 6
      q_t in_q,        // to receive from terminal
          out_q,       // to transmit to terminal
          echo_q;      // to echo back to terminal
      int in_q_sem,    // receive data (arrived) count
          out_q_sem,   // transmit space available count
          flag,        // echo back to terminal or not
          out_extra;   // if 1, TXRDY event occurred while echo_q and out_q both empty
}interface_t;


typedef struct{
	int pid;//pid that uses RAM page
	int addr;//location of 4KB RAM
}page_info_t;

typedef void (*func_ptr_t)(); // void-return function pointer type

#endif _TYPE_H_
