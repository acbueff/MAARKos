// isr.c, 159

#include "spede.h"
#include "type.h"
#include "isr.h"
#include "tool.h"
#include "extern.h"
#include "proc.h"

void NewProcISR(){
	int new_pid;
   /**check unused_q if no ID left
      show msg on PC: "Kernel Panic: no more process ID left!\n"
      return;*/
	 if(unused_q.size == 0){ //keep in mind proc 0 should always run 
		cons_printf("Kernel Panic: no more process ID left!\n");
		return;
	 }

   /**dequeue unused_q for a new pid*/
   
   new_pid = DeQ(&unused_q);
   /**set PCB of new process (indexed by pid):
   reset both time and total_time
   state is set to READY*/
   pcb[new_pid].state = READY;
   pcb[new_pid].time = 0;
   pcb[new_pid].total_time = 0;
   

   /**enqueue this new pid into process queue*/
   EnQ(new_pid, &proc_q);
   
   //following is to build proc trapframe in its runtime stack
   MyBzero(stack[new_pid],STACK_SIZE);
   // position to frame it against top of stack, then fill it out
   pcb[new_pid].trapframe_p = (trapframe_t *)
      &stack[new_pid][STACK_SIZE-sizeof(trapframe_t)];

   if(new_pid == 0){
      pcb[new_pid].trapframe_p->eip = (unsigned int)InitProc;   // InitProc process
   }
   if(new_pid == 1){
      pcb[new_pid].trapframe_p->eip = (unsigned int)PrintDriver;   // other processes
   }
   if(new_pid == 2){
      pcb[new_pid].trapframe_p->eip = (unsigned int)UserShell;   // other processes
   }
   if(new_pid == 3){
      pcb[new_pid].trapframe_p->eip = (unsigned int)TerminalOut;   // other processes
   }
   if(new_pid == 4){
      pcb[new_pid].trapframe_p->eip = (unsigned int)TerminalIn;   // other processes
   }

/**	else{
		pcb[new_pid].trapframe_p->eip = (unsigned int)InitProc;
	}*/
   pcb[new_pid].trapframe_p->eflags = EF_DEFAULT_VALUE|EF_INTR; // set INTR flag
   pcb[new_pid].trapframe_p->cs = get_cs();                     // standard fair
   pcb[new_pid].trapframe_p->ds = get_ds();                     // standard fair
   pcb[new_pid].trapframe_p->es = get_es();                     // standard fair
   pcb[new_pid].trapframe_p->fs = get_fs();                     // standard fair
   pcb[new_pid].trapframe_p->gs = get_gs();                     // standard fair
   
}

void ProcExitISR() {
   /**just return if running PID is 0 (InitProc does not exit)*/
   if(run_pid == 0){
		return;
   }
      
   /**change state of running process to UNUSED
   queue running PID back to unused queue
   set running PID to -1 (now not given)*/
   pcb[run_pid].state = UNUSED;
   EnQ(run_pid, &unused_q);
   run_pid = -1;
}

void TimerSub(){
	int i;
	int sleepPID;
	if(sleep_q.size != 0){
		for(i = 0; i < sleep_q.size; i++){
		sleepPID = DeQ(&sleep_q);
		if(system_time >= pcb[sleepPID].wake_time){
			EnQ(sleepPID, &proc_q);
			pcb[sleepPID].state = READY;
		}
		else{
			EnQ(sleepPID,&sleep_q);
		}
		}
	}
	
}        

void TimerISR() {

   /**just return if running PID is -1 (not a valid PID)
   (shouldn't happen, Kernel Panic is more appropriate)*/
   if(run_pid == -1){
	cons_printf("Kernel Panic - TimerISR\n");
	return;
   }
   system_time++;
   TimerSub();
   /**upcount the CPU time of running process*/
   pcb[run_pid].time++;

  
   if(pcb[run_pid].time == T_SLICE){ // the time counted has reached to be T_SLICE:
		//(should rotate to next PID in process queue)
		pcb[run_pid].total_time = pcb[run_pid].time + pcb[run_pid].total_time;//sum up time to total time in PCB of running process
		pcb[run_pid].time = 0;//reset
		pcb[run_pid].state = READY; //change its state to READY
		EnQ(run_pid, &proc_q);//queue it to process queue
		run_pid = -1;//set running PID to -1 (gone, Scheduler() will pick a new one)
	}
	     
      
}

void GetPidISR(){
	/**copy current running PID to the right field in the trapframe
   of the current running process.*/
   pcb[run_pid].trapframe_p->eax = run_pid;
}

void GetTimeISR(){
	pcb[run_pid].trapframe_p->eax = system_time;
}

void SleepISR(){
	/** calculate the wake time for the process,*/
	int sleep = pcb[run_pid].trapframe_p->eax;
	pcb[run_pid].wake_time = system_time + sleep*100;
	/**
	queue it to the sleep queue, change state, reset current running PID.
	*/
	EnQ(run_pid, &sleep_q);
	pcb[run_pid].state = SLEEP;
	run_pid = -1;//?reset
}

void SemGetISR(){
	int semID;
	if(semaphore_q.size == 0){
		pcb[run_pid].trapframe_p->eax = -1;
	}else{
		semID = DeQ(&semaphore_q);
		pcb[run_pid].trapframe_p->eax = semID;
		MyBzero((char *)&semaphore[semID],sizeof(semaphore[semID]));
	}
}
void SemPostISR(int sem_num){
	//int pid = pcb[run_pid].trapframe_p->eax;
	int waitPid;
	if(semaphore[sem_num].wait_q.size == 0){
		semaphore[sem_num].pass_count++;
	}
	else{
	//cons_printf("!!!SemPostIsR: freeing process # %d !!!\n",run_pid);
		waitPid = DeQ(&(semaphore[sem_num].wait_q));
		EnQ(waitPid, &proc_q);
		pcb[waitPid].state = READY;
	}
}
void SemWaitISR(){
	int pid = pcb[run_pid].trapframe_p->eax;
	if(semaphore[pid].pass_count > 0){
		semaphore[pid].pass_count--;
		
	}
	else{
		//cons_printf("!!!SemWaitIsR: blocking process # %d !!!\n",run_pid);
		EnQ(run_pid, &(semaphore[pid].wait_q));
		pcb[run_pid].state = WAIT;
		run_pid = -1;
		
	}
}

void MsgSndISR(){
	int x; 	//mailbox#
	int freed; //PID
	msg_t *source_p;
	msg_t *destination_p;
	
	//get x from the register in the trapframe passed over
	x = pcb[run_pid].trapframe_p->eax;
	source_p = (msg_t *)pcb[run_pid].trapframe_p->ebx;
	//update source_p
	source_p->sender = run_pid;//or run_pid
	source_p->time_stamp=system_time;//system time?
	if(mbox[x].wait_q.size == 0){
		MsgEnQ(source_p,&mbox[x]);
	}else{
		freed = DeQ(&mbox[x].wait_q);
		EnQ(freed,&proc_q);
		pcb[freed].state = READY;
		//copy incoming message over to own message space:
		//use source_p and destination_p;
		destination_p = (msg_t *)pcb[freed].trapframe_p->ebx;
		*destination_p = *source_p;
		
	}
	
}

void MsgRcvISR(){
	int x;
	msg_t *source_p;
	msg_t *destination_p;
	
	x = pcb[run_pid].trapframe_p->eax;
	destination_p = (msg_t *)pcb[run_pid].trapframe_p->ebx;
	
	if(mbox[x].size != 0){
		source_p = (msg_t *)MsgDeQ(&mbox[x]);
		*destination_p = *source_p;
	}else{
		//block run_pid
		EnQ(run_pid, &(mbox[x].wait_q));
		pcb[run_pid].state = WAIT;
		run_pid = -1;
	
	}
}


void PrinterISR(){
	//int pid = pcb[run_pid].trapframe_p->eax;
	int waitPid;
	if(semaphore[system_print_semaphore].wait_q.size == 0){
		semaphore[system_print_semaphore].pass_count++;
	}
	else{
	//cons_printf("!!!PrinterIsR: freeing process # %d !!!\n",run_pid);
		waitPid = DeQ(&(semaphore[system_print_semaphore].wait_q));
		EnQ(waitPid, &proc_q);
		pcb[waitPid].state = READY;
	}
}

void TerminalISR(){
	char status;
	status = inportb(COM2_IOBASE+IIR);
	switch(status){
		case IIR_TXRDY:
			TerminalISRout();
			break;
		case IIR_RXRDY:
			TerminalISRin();
			break;
		default:
			break;
	}
	if(proc_interface.out_extra == 1){
		TerminalISRout();
	}
}

void TerminalISRout(){
	// dequeue out_q to write to port
      char ch = 0; // NUL, '\0'
 
      if(proc_interface.echo_q.size != 0){//echo queue of the interface is not empty {
         ch = DeQ(&proc_interface.echo_q);//dequeue from echo queue of the interface
      } else {
         if(proc_interface.out_q.size != 0){//out queue of the interface is not empty
            ch = DeQ(&proc_interface.out_q);//dequeue from out queue of the interface
            SemPostISR( proc_interface.out_q_sem);
		}
	 }
      if(ch != 0){
         //use outportb() to send ch to COM2_IOBASE+DATA
         outportb(ch,COM2_IOBASE+DATA);
		 //out_extra is cleared (to 0)
		//MyBzero(proc_interface.out_extra);
		proc_interface.out_extra = 0;
	  } else {
         //out_extra is set to 1 (cannot use it now since no char)
		proc_interface.out_extra = 1;
	  }

}

void TerminalISRin(){
	char ch;
 
      // use 127 to mask out msb (rest 7 bits in ASCII range)
      ch = inportb(COM2_IOBASE+DATA) & 0x7F;  // mask 0111 1111
 
      //enqueue ch to in_q of the interface
      EnQ(ch,&proc_interface.in_q);
	  SemPostISR(proc_interface.in_q_sem);
 
      if(ch =='\r' ){
        // enqueue '\r' then '\n' to echo_q of the interface
		EnQ('\r',&proc_interface.echo_q);
		EnQ('\n',&proc_interface.echo_q);
	  } else {
         if(proc_interface.flag == 1){ //echo of the interface is 1 {
            //enqueue ch to echo_q of the interface
				EnQ(ch,&proc_interface.echo_q);
		 }
      }

}
	
