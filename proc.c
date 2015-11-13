// proc.c, 159
// processes are coded here

#include "spede.h"      // for IO_DELAY() needed here below
#include "extern.h"     // for current_run_pid needed here below
#include "proc.h"       // for prototypes of process functions
#include "syscall.h"
#include "type.h"
#include "tool.h"

void InitProc() {
	int i;
   //show msg on PC:
   char key;
   char *msg = "Hello World! Team MAARK standing by\n";
   msg_t my_msg;

   /**use a busy-loop to delay for 1 sec*/
   while(1){
    cons_printf("InitProc runs\n");
	for(i = 0; i<1666000; i++) IO_DELAY(); //only .65 micro

	if(cons_kbhit()){  //a key has been pressed on PC
		key = cons_getchar();
		switch(key) {
			 case 'b':
				breakpoint(); //to go into GDB
				break;
			 case 'p':
				MyStrcpy(my_msg.data,msg);
				MsgSnd(1,&my_msg);
				break;
			 case 'q':
				exit(0); //to quit MyOS.dli
     } // end switch
   } // end if key pressed

   }
}

void UserProc() {
	int ownPID;
	int sysTime;
	int sleep;//sleep_seconds
   while(1){
   ownPID = GetPid();
   sysTime = GetTime();
   sleep = (ownPID % 5) + 1;
   //cons_printf("UserProc # %d runs, got time = %d, sleep %d seconds\n",ownPID,sysTime,sleep);
   Sleep(sleep);

   }
}



void PrintDriver() {
   int i, code;
   msg_t my_msg;
   char *p;
	int ownPID =  GetPid();
   //get a semaphore for the new global int system_print_semaphore
	system_print_semaphore = SemGet();
// reset printer (check printer power, cable, and paper), it will jitter
   outportb(LPT1_BASE+LPT_CONTROL, PC_SLCTIN);   // CONTROL reg, SeLeCT INterrupt
   code = inportb(LPT1_BASE+LPT_STATUS);         // read STATUS
   //loop 50 times of IO_DELAY();                  // needs delay
   for(i = 0; i<50; i++) IO_DELAY();
   outportb(LPT1_BASE+LPT_CONTROL, PC_INIT|PC_SLCTIN|PC_IRQEN); // IRQ ENable
   //Sleep for a second                            // needs time resetting
   Sleep(1);
   SemWait(system_print_semaphore);
   while(1){
      //cons_printf my PID                         // show itself (see demo)
      cons_printf("PrinterDriver # %d runs\n",ownPID);
	  MsgRcv(ownPID, &my_msg);                          // get if msg to print
      //set p to point to start of character string in message
	  p = &my_msg.data[0];
      while( *p!= '\0'){
         outportb(LPT1_BASE+LPT_DATA, *p);       // write char to DATA reg
         code = inportb(LPT1_BASE+LPT_CONTROL);  // read CONTROL reg
         outportb(LPT1_BASE+LPT_CONTROL, code|PC_STROBE); // write CONTROL, STROBE added
         for(i = 0; i<50; i++) IO_DELAY();              // needs delay
         outportb(LPT1_BASE+LPT_CONTROL, code);  // send back original CONTROL
        //wait on the system printing semaphore
		SemWait(system_print_semaphore);
		p++;
         //move pointer p to next character in message string
      }//end while
   }//end forever loop
}//end PrintDriver()


void UserShell(){
   msg_t msg;
   char something[101]; // a handy string
   int BAUD_RATE, divisor,               // serial port use
       i, my_pid, // size,
       TerminalInPid = 3,                // helper: TerminalIn process
       TerminalOutPid = 4;               // helper: TerminalOut process
   //initialize the interface data structure:
   MyBzero((char*)&proc_interface,sizeof(proc_interface));
   proc_interface.out_q_sem = SemGet();//==1
   proc_interface.in_q_sem = SemGet();//==2
   for(i = 0; i < Q_SIZE;i++){
		SemPost(proc_interface.out_q_sem);
   }
   proc_interface.flag = 1;
   proc_interface.out_extra = 1;
   //reset the serial port:
   // A. set baud rate 9600
   BAUD_RATE = 9600;              // Mr. Baud invented this
   divisor = 115200 / BAUD_RATE;  // time period of each baud
   outportb(COM2_IOBASE+CFCR, CFCR_DLAB);          // CFCR_DLAB 0x80
   outportb(COM2_IOBASE+BAUDLO, LOBYTE(divisor));
   outportb(COM2_IOBASE+BAUDHI, HIBYTE(divisor));
   // B. set CFCR: 7-E-1 (7 data bits, even parity, 1 stop bit)
   outportb(COM2_IOBASE+CFCR, CFCR_PEVEN|CFCR_PENAB|CFCR_7BITS);
   outportb(COM2_IOBASE+IER, 0);
   // C. raise DTR, RTS of the serial port to start read/write
   outportb(COM2_IOBASE+MCR, MCR_DTR|MCR_RTS|MCR_IENABLE);
   IO_DELAY();
   outportb(COM2_IOBASE+IER, IER_ERXRDY|IER_ETXRDY); // enable TX, RX events
   IO_DELAY();

   my_pid = GetPid();

   msg.sender = my_pid;

   MyStrcpy(msg.data,"\nHello World! Team MAARK is here!\n");
   MsgSnd(TerminalOutPid,&msg);
   MsgRcv(my_pid,&msg);

  while(1){

      //(send msg to TerminalOutPid, receive its reply)
	  MyStrcpy(msg.data,"\nEnter something->");
	  MsgSnd(TerminalOutPid,&msg);
	  MsgRcv(my_pid,&msg);

      //get what's entered
	  MyStrcpy(something,msg.data);
	  MsgSnd(TerminalInPid,&msg);
	  MsgRcv(my_pid,&msg);

	  MyStrcpy(something,msg.data);

	  MyStrcpy(msg.data,"You've entered ->");
	  MsgSnd(TerminalOutPid,&msg);
	  MsgRcv(my_pid,&msg);

	  //(send msg to TerminalOutPid, receive its reply)
	  MyStrcpy(msg.data,something);
	  MsgSnd(TerminalOutPid,&msg);
	  MsgRcv(my_pid,&msg);
   }//end forever loop
}

void TerminalIn(){
	char *p;
	char ch;
	msg_t msg;
	int my_pid = GetPid();
	while(1){
		MsgRcv(my_pid,&msg);
		p = msg.data;
		while(1){
			SemWait(proc_interface.in_q_sem);
			ch = (char)DeQ(&proc_interface.in_q);
			if(ch == '\r'){break;}
			*p++ = ch;
			//ch = *p;
		}
		*p='\0';
		MsgSnd(msg.sender,&msg);
	}
}

void TerminalOut(){
	char *p;
	msg_t msg;
	int my_pid = GetPid();
	while(1){
		MsgRcv(my_pid,&msg);
		p = msg.data;
		while(*p != '\0'){
			SemWait(proc_interface.out_q_sem);
			EnQ((int)*p,&proc_interface.out_q);
			TripTerminal();
			if(*p=='\n'){
				SemWait(proc_interface.out_q_sem);
				EnQ((int)'\r',&proc_interface.out_q);
			}
			p++;
		}

		MsgSnd(msg.sender,&msg);
	}
}
