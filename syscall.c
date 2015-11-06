// syscall.c
// collection of syscalls, i.e., API
#include "type.h" 
 
int GetPid() {                   // no input, has return
   int x;
 
   asm("int $48; movl %%eax, %0" // CPU inst
       : "=g" (x)                // output from asm("...")
       :                         // no input into asm("...")
       : "%eax");                // push before asm("..."), pop after
 
   return x;
}
 
// ...
// ??? need to code GetTime() here
// ...
int GetTime(){
	int x;
	/**Not sure if correct*/
	asm("int $49; movl %%eax, %0"
		: "=g" (x)
		:
		: "%eax"); //edx
		
	return x;
}
 
void Sleep(int sleep_seconds) {  // has input, no return
 
   asm("movl %0, %%eax; int $50" // CPU inst
       :                         // no output from asm("...")
       : "g" (sleep_seconds)     // input into asm("...")
       : "%eax");                // push before asm("..."), pop after
}

int SemGet(){
	int x;
	asm("int $51; movl %%eax, %0"
		: "=g" (x)
		:
		: "%eax");
	
	return x;
}

void SemPost(int semaphore_num){

	asm("movl %0, %%eax; int $52"
		:
		: "g" (semaphore_num)
		: "%eax");
}	

void SemWait(int semaphore_num){

	asm("movl %0, %%eax; int $53"
		:
		: "g" (semaphore_num)
		: "%eax");
}	

void MsgSnd(int y, msg_t *msg_t){

	asm("movl %0,%%eax; movl %1, %%ebx; int $54"
		:
		:"g"(y),"g"((int)msg_t)
		:"%eax","%ebx");
	
}

void MsgRcv(int y, msg_t *msg_t){

		asm("movl %0, %%eax;movl %1, %%ebx; int $55"
		:
		:"g"(y),"g"((int)msg_t)
		:"%eax","%ebx");
		
}

void TripTerminal(){
	asm("int $35");
}


