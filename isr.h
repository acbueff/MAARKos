// isr.h, 159

#ifndef _ISR_H_
#define _ISR_H_

void NewProcISR();
void ProcExitISR();
void TimerISR();
void GetTimeISR();
void GetPidISR();
void SleepISR();
void TimerSub();
void SemGetISR();
void SemPostISR(int sem_num);
void SemWaitISR();
void MsgSndISR();
void MsgRcvISR();
void PrinterISR();
void TerminalISR();
void TerminalISRout();
void TerminalISRin();



#endif
