// entry.h, 159

#ifndef _ENTRY_H_
#define _ENTRY_H_

#include <spede/machine/pic.h>

#define TIMER_INTR 32
#define PRINTER_INTR 39
#define TERMINAL_INTR 35

#define KCODE 0x08         // kernel's code segment
#define KDATA 0x10         // kernel's data segment
#define KSTACK_SIZE 16384  // kernel's stack byte size
#define GETPID_INTR 48
#define GETTIME_INTR 49
#define SLEEP_INTR 50
#define SEMGET_INTR 51
#define SEMPOST_INTR 52
#define SEMWAIT_INTR 53
#define MSGSND_INTR 54
#define MSGRCV_INTR 55

#define FORK_INTR 56
#define WAIT_INTR 57
#define EXIT_INTR 58

// ISR Entries
#ifndef ASSEMBLER

__BEGIN_DECLS

 void TimerEntry();     // code defined in entry.S
 void Loader();         // code defined in entry.S
 void GetPidEntry();
 void GetTimeEntry();
 void SleepEntry();
  //3 new prototypes
 void SemGetEntry();
 void SemPostEntry();
 void SemWaitEntry();
 //PHASE 4
 void MsgSndEntry();
 void MsgRcvEntry();
 //PHASE 5
 void PrinterEntry();
 void TerminalEntry();
 //PHASE 8
 void ForkEntry();
 void WaitEntry();
 void ExitEntry();

__END_DECLS

#endif

#endif
