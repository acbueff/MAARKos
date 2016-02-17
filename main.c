// main.c, 159
// kernel is only simulated
// Phase 8
// Team Name: MAARK (Members: Andreas Bueff, Mike Smith)

#include "spede.h"      // spede stuff
#include "main.h"       // main stuff
#include "isr.h"        // ISR's
#include "tool.h"       // handy functions for Kernel
#include "proc.h"       // processes such as InitProc()
#include "type.h"       // processes such as InitProc()

#include "entry.h"       // new one for trapframe--in order to locate TimerEntry to set IDT

// kernel data stuff:
int run_pid;   // current running PID, if -1, no one running

int system_time;
int kernel_MMU_addr;
q_t proc_q, unused_q, sleep_q,semaphore_q;   // processes ready to run and ID's un-used
pcb_t pcb[MAX_PROC];    // process table
mbox_t mbox[MAX_PROC];
char stack[MAX_PROC][STACK_SIZE]; // runtime stacks of processes
semaphore_t semaphore[Q_SIZE];
//int product_semaphore, product_count;
int system_print_semaphore;
interface_t proc_interface;
page_info_t page_info[MAX_PROC*5];
struct i386_gate *IDT_ptr;

int main() {

	SetData();
	SetControl();
	NewProcISR();//create init proc
	NewProcISR();//create printdriver
	NewProcISR();//user
	NewProcISR();//termin
	NewProcISR();//termout
	NewProcISR();//filesys
	Scheduler();
	Loader(pcb[run_pid].trapframe_p);

   return 0;
}

void SetData() {
	int i;
	system_time = 0;
	//product_mbox_num = 0;
   system_print_semaphore = 0;
   kernel_MMU_addr = get_cr3();//PHASE9

   MyBzero((char*)&proc_q,sizeof(proc_q));   //set process to run queue to all 0s
   MyBzero((char*)&unused_q,sizeof(unused_q)); //set unused processes queue to all 0s
   MyBzero((char*)&sleep_q,sizeof(sleep_q));
   MyBzero((char*)&semaphore_q,sizeof(semaphore_q));
   MyBzero((char*)&mbox[0],sizeof(mbox_t));
  for(i = 0; i < MAX_PROC*5;i++){
   page_info[i].pid = -1;
   page_info[i].addr = (i*0x1000) + 0xE00000;//NOT SURE
  }
   /**queue ID's 0~19 into unused_q*/
   for(i = 0; i < Q_SIZE; i++){
	EnQ(i,&unused_q);
	EnQ(i,&semaphore_q);
	pcb[i].state = UNUSED;
   }

   run_pid = -1;  /**(not given, need to chose a process to run, by Scheduler())*/
}

void Scheduler() {            // choose run PID
   /**simply return if run_pid is not -1 (would it be other negative #?)*/
   if( run_pid != -1){
	return;
   }

	if(proc_q.size != 0){
		run_pid = DeQ(&proc_q);
		pcb[run_pid].state = RUN;
	}
	else{//proc_q was empty
		cons_printf("Kernel Panic: not process to run!\n");
		breakpoint();//might not exist
	}
}

/**from timer lab*/
void SetEntry(int entry_num, func_ptr_t func_ptr){
	struct i386_gate *gateptr = &IDT_ptr[entry_num];
	fill_gate(gateptr, (int)func_ptr, get_cs(), ACC_INTR_GATE,0);
}


void SetControl(){
/**
contains 3 statements learned from the timer lab:
   (but NO "sti" since it will be given from a trapframe to be loaded)
*/
	IDT_ptr = get_idt_base(); // locate IDT
	//program PIC mask//153
	SetEntry(32, TimerEntry);//fill out IDT timer entry (for handling of timer interrupts)
	SetEntry(35, TerminalEntry);
	SetEntry(39, PrinterEntry);//FIX!!

	/**add new IDT entires----not correct yet*/
	SetEntry(48, GetPidEntry);
	SetEntry(49, GetTimeEntry);
	SetEntry(50, SleepEntry);
	SetEntry(51, SemGetEntry);
	SetEntry(52, SemPostEntry);
	SetEntry(53, SemWaitEntry);
	SetEntry(54, MsgSndEntry);
	SetEntry(55, MsgRcvEntry);
	SetEntry(56, ForkEntry);
	SetEntry(57, WaitEntry);
	SetEntry(58, ExitEntry);
	outportb(0x21, ~137);
}



void Kernel(trapframe_t *p) {
   /**save p into the PCB of the running process*/

   pcb[run_pid].trapframe_p = p;
   switch(p->intr_num){
		case TIMER_INTR:
			TimerISR();
			outportb(0x20,0x60);	//dismiss timer event with PIC (timer lab)
			break;
		case PRINTER_INTR:
			PrinterISR();
			outportb(0x20,0x67);//check
			break;
		case TERMINAL_INTR:
			TerminalISR();
			outportb(0x20,0x63);//check
			break;
		case GETPID_INTR:
			GetPidISR();
			break;
		case GETTIME_INTR:
			GetTimeISR();
			break;
		case SLEEP_INTR:
			SleepISR();
			break;
		case SEMGET_INTR:
			SemGetISR();
			break;
		case SEMPOST_INTR:
			SemPostISR(p->eax);
			break;
		case SEMWAIT_INTR:
			SemWaitISR();
			break;
		case MSGSND_INTR:
			MsgSndISR();
			break;
		case MSGRCV_INTR:
			MsgRcvISR();
			break;
		case FORK_INTR:
			ForkISR();
			break;
		case WAIT_INTR:
			WaitISR();
			break;
		case EXIT_INTR:
			ExitISR();
			break;
		default:
			cons_printf("Kernel Panic: no such intr # (%d)!\n", p->intr_num);
			breakpoint();
   }




   Scheduler();// to choose next running process if needed
   set_cr3(pcb[run_pid].MMU_addr);
   Loader(pcb[run_pid].trapframe_p);
}

