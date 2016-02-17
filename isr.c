// isr.c, 159

#include "spede.h"
#include "type.h"
#include "isr.h"
#include "tool.h"
#include "extern.h"
#include "proc.h"
//#include "FileSystem.h"

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



   switch(new_pid){
	   case 0:
		  pcb[new_pid].trapframe_p->eip = (unsigned int)InitProc;   // InitProc process
		  break;
	   case 1:
		  pcb[new_pid].trapframe_p->eip = (unsigned int)PrintDriver;   // other processes
		  break;
	   case 2:
		  pcb[new_pid].trapframe_p->eip = (unsigned int)UserShell;   // other processes
		  break;
	   case 3:
		  pcb[new_pid].trapframe_p->eip = (unsigned int)TerminalIn;   // other processes
		  break;
	   case 4:
		  pcb[new_pid].trapframe_p->eip = (unsigned int)TerminalOut;   // other processes
	      break;
	   case 5:
		  pcb[new_pid].trapframe_p->eip = (unsigned int)FileSystem;   // other processes
		   break;
		default:
			break;
   }

/**	else{
		pcb[new_pid].trapframe_p->eip = (unsigned int)InitProc;
	}*/
   pcb[new_pid].MMU_addr = kernel_MMU_addr;

   pcb[new_pid].trapframe_p->eflags = EF_DEFAULT_VALUE|EF_INTR; // set INTR flag
   pcb[new_pid].trapframe_p->cs = get_cs();                     // standard fair
   pcb[new_pid].trapframe_p->ds = get_ds();                     // standard fair
   pcb[new_pid].trapframe_p->es = get_es();                     // standard fair
   pcb[new_pid].trapframe_p->fs = get_fs();                     // standard fair
   pcb[new_pid].trapframe_p->gs = get_gs();                     // standard fair

}

void ProcExitISR() {
	set_cr3(kernel_MMU_addr);
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
    set_cr3(kernel_MMU_addr);
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
		set_cr3(kernel_MMU_addr);
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
	set_cr3(kernel_MMU_addr);
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
	int waitPid;
	if(semaphore[sem_num].wait_q.size == 0){
		semaphore[sem_num].pass_count++;
	}
	else{
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
		EnQ(run_pid, &(semaphore[pid].wait_q));
		pcb[run_pid].state = WAIT;
		run_pid = -1;
		set_cr3(kernel_MMU_addr);
	}
}

void MsgSndISR(){
	int x; 	//mailbox#
	int freed; //PID
	msg_t *source_p;
	msg_t *destination_p;
	msg_t message;
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
		pcb[freed].state = RUN;

		message = *source_p;
		//copy incoming message over to own message space:
		//use source_p and destination_p;
		set_cr3(pcb[freed].MMU_addr);
		destination_p = (msg_t *)pcb[freed].trapframe_p->ebx;
		*destination_p = message;
		set_cr3(pcb[run_pid].MMU_addr);
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
		set_cr3(kernel_MMU_addr);
	}
}


void PrinterISR(){

	int waitPid;
	if(semaphore[system_print_semaphore].wait_q.size == 0){
		semaphore[system_print_semaphore].pass_count++;
	}
	else{
		waitPid = DeQ(&(semaphore[system_print_semaphore].wait_q));
		EnQ(waitPid, &proc_q);
		pcb[waitPid].state = READY;
	}
}

void TerminalISR(){
	int status;
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
	char ch = 0;
	if(proc_interface.echo_q.size != 0){//echo queue of the interface is not empty {
         ch =DeQ(&proc_interface.echo_q);//dequeue from echo queue of the interface
	  } else {
         if(proc_interface.out_q.size != 0){//out queue of the interface is not empty
            ch = DeQ(&proc_interface.out_q);//dequeue from out queue of the interface
			SemPostISR( proc_interface.out_q_sem);
		}
	 }
      if(ch != 0){
        outportb(COM2_IOBASE+DATA,ch);
		proc_interface.out_extra = 0;
	  } else {
		proc_interface.out_extra = 1;
	  }

}

void TerminalISRin(){
	  char ch;

      ch = inportb(COM2_IOBASE+DATA) & 0x7F;  // mask 0111 1111

      EnQ(ch,&proc_interface.in_q);
	  SemPostISR(proc_interface.in_q_sem);

      if(ch =='\r' ){
		EnQ(ch,&proc_interface.echo_q);
		EnQ('\n',&proc_interface.echo_q);
	  } else {
         if(proc_interface.flag == 1){
				EnQ(ch,&proc_interface.echo_q);
		 }
      }

}

void ForkISR(){
	int j =0;
	int pid;
	int page_num, executable_addr,
       get_them[5],    // need 5 free page indices
       *p,             // to fill table entries
       main_table, code_table, stack_table, code_page, stack_page,
       i;              // to loop
	i = 0;
	if(unused_q.size == 0){
		cons_printf( "Kernel Panic: no available PID!\n");
		return;
	}
	while(1){
		if(page_info[j].pid == -1){
			page_num = j;
			get_them[i] =page_num;
			i++;
			if(i == 5){break;}
		}

		if(j == (MAX_PROC*5)-1){
			cons_printf( "Kernel Panic: no available RAM page!\n");
			return;
		}
		j++;
	}

	if(i != 5){
			cons_printf("Kernel Panic: no five RAM pages available\n");
			return;
	 }
	pid = DeQ(&unused_q);
	for(i = 0; i < 5; i++){
		page_info[get_them[i]].pid = pid;
		MyBzero((char*)page_info[get_them[i]].addr,4096);
	}

	main_table = page_info[get_them[0]].addr;
	code_table = page_info[get_them[1]].addr;
	stack_table = page_info[get_them[2]].addr;
	code_page = page_info[get_them[3]].addr;
	stack_page = page_info[get_them[4]].addr;

	MyMemcpy((char*)main_table, (char*)kernel_MMU_addr,4*4);
	p = (int*)main_table;
	p[512]  = code_table | 3;
	p[1023] = stack_table | 3;
	page_info[page_num].pid = pid;
	p = (int*)code_table;
	p[0] = code_page | 3;
	p = (int*) stack_table;
	p[1023] = stack_page | 3;
	executable_addr = pcb[run_pid].trapframe_p->eax;
	MyMemcpy((char*)code_page,(char*)executable_addr,4096);

	//set "pid" info og allocated ram page t
	MyBzero((char*)&mbox[pid], sizeof(mbox_t));
	EnQ(pid,&proc_q);
	//set PCB
	pcb[pid].MMU_addr = main_table;
	pcb[pid].time = 0;
	pcb[pid].total_time = 0;
	pcb[pid].state =READY;
	pcb[pid].ppid = run_pid;//set to calling process
	//MyMemcpy((char*)page_info[page_num].addr,(char*)pcb[run_pid].trapframe_p->eax,4096);//to RAM page
	pcb[pid].trapframe_p = (trapframe_t *)(stack_page + 4096-sizeof(trapframe_t));//begin of 4KB page +128...BEFORE page_info[page_num].addr
	pcb[pid].trapframe_p->eip = 0x80000080;
   pcb[pid].trapframe_p->eflags = EF_DEFAULT_VALUE|EF_INTR; // set INTR flag
   pcb[pid].trapframe_p->cs = get_cs();                     // standard fair
   pcb[pid].trapframe_p->ds = get_ds();                     // standard fair
   pcb[pid].trapframe_p->es = get_es();                     // standard fair
   pcb[pid].trapframe_p->fs = get_fs();                     // standard fair
   pcb[pid].trapframe_p->gs = get_gs();
  //(unsigned int)page_info[page].addr+128;
   pcb[pid].trapframe_p = (trapframe_t *)0xffffffc0;
 }

void WaitISR(){
	int child_pid;
	int *x;
	//int *exit_num;
	int i;
	int page_num = 0;
	int found = 0;
	for(i = 0; i < MAX_PROC; i++){
		if((pcb[i].ppid == run_pid) && (pcb[i].state ==ZOMBIE)){
			found = 1;
			child_pid = i;
			pcb[run_pid].trapframe_p->ebx = child_pid;
			x = (int*)pcb[run_pid].trapframe_p->eax;
			*x = pcb[child_pid].trapframe_p->eax;
			while(1){
				if(page_info[page_num].pid == child_pid){
					MyBzero((char*)&page_info[page_num].addr, 4096);
					page_info[page_num].pid = -1;
					pcb[child_pid].state = UNUSED;
					EnQ(child_pid,&unused_q);
					break;
				}
				else if(page_num == (MAX_PROC*5)-1){// && page_info[page_num].pid != child_pid){
					cons_printf( "Kernel Panic: no available RAM page!\n");
					return;
				}
				page_num++;
			}
		}
	}
	if(!found){
			pcb[run_pid].state = WAIT4CHILD;
			run_pid = -1;
			set_cr3(kernel_MMU_addr);
			return;
	}
	//recycle resources


}

void ExitISR(){
	int i=0;
	int page;
	int *x;
	int *exit_num;
	int ppid = pcb[run_pid].ppid;
	set_cr3(kernel_MMU_addr);
	if(pcb[ppid].state != WAIT4CHILD){
		pcb[run_pid].state = ZOMBIE;
		run_pid = -1;

		return;
	}
	else{
		//release parent
		pcb[ppid].state = READY;
		EnQ(ppid, &proc_q);
		pcb[ppid].trapframe_p->ebx = run_pid;
		exit_num = (int*)pcb[run_pid].trapframe_p->eax;
		x = (int*)pcb[ppid].trapframe_p->eax;
		*x = (int)exit_num;

	}
	//recycle calling process pages
	while(1){
		if(page_info[i].pid == run_pid){
			page = i;
			break;
		}
		if(i == (MAX_PROC*5)-1 ){
			cons_printf( "Kernel Panic: no available RAM page!\n");
			return;
		}
		i++;
	}
	page_info[page].pid = -1;
	MyBzero((char*)&page_info[page], sizeof(page_info[page]));
	pcb[run_pid].state = UNUSED;
	EnQ(run_pid,&unused_q);
	run_pid =-1;
}


