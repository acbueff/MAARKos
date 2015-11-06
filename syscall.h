// syscall.h
 
#ifndef _SYSCALL_H_
#define _SYSCALL_H_
 #include "type.h" 
int GetPid(void);  // no input, 1 return
int GetTime(void); 
void Sleep(int);   // 1 input, no return
int SemGet(void);
void SemPost(int);
void SemWait(int);
void MsgSnd(int, msg_t *);
void MsgRcv(int, msg_t *);
void TripTerminal(void);


#endif