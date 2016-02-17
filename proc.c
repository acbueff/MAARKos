// proc.c, 159
// processes are coded here

#include "spede.h"      // for IO_DELAY() needed here below
#include "extern.h"     // for current_run_pid needed here below
#include "proc.h"       // for prototypes of process functions
#include "syscall.h"
#include "type.h"
#include "tool.h"
//#include "FileSystem.h"
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
   attr_t *ptr;
   char something[101]; // a handy string
   char login[101],password[101];

   int BAUD_RATE, divisor,               // serial port use
       i, my_pid, exit_num,child_pid,// size,
          TerminalInPid = 3,                // helper: TerminalIn process
       TerminalOutPid = 4,               // helper: TerminalOut process
       FileSystemPid = 5;                // File System PID 5               // helper: TerminalOut process
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

   MyStrcpy(msg.data, "\n\n\n\nHello, World! MAARK here!\n\n");
   MsgSnd(TerminalOutPid, &msg);        // prompt this
   MsgRcv(my_pid, &msg);                // stop/go sync, content ignored

   MyStrcpy(msg.data, "After login, commands are: dir [path], cat (file), out\n");
   MsgSnd(TerminalOutPid, &msg);
   MsgRcv(my_pid, &msg);

   MyStrcpy(msg.data, "(or use 111/222/000 as above if keyboard MISSING keys.)\n\n\0");
   MsgSnd(TerminalOutPid, &msg);
   MsgRcv(my_pid, &msg);


  while(1){
	while(1){
		//prompt
		MyStrcpy(msg.data, "(MAARK) login -> ");
		MsgSnd(TerminalOutPid,&msg);
		MsgRcv(my_pid,&msg);
		//get what's entered(login)

		MsgSnd(TerminalInPid,&msg);
		MsgRcv(my_pid,&msg);
		MyStrcpy(login,msg.data);
		//prompt
		MyStrcpy(msg.data,"(MAARK) password -> ");
		MsgSnd(TerminalOutPid,&msg);
		MsgRcv(my_pid,&msg);
		//turn echo flag off in interface
		proc_interface.flag = 0;
		//get what's entered(password)
		MsgSnd(TerminalInPid,&msg);
		MsgRcv(my_pid,&msg);
		MyStrcpy(password,msg.data);
		//compare password
		if(MyStrcmp(password,login, MyStrlen(login))){
			break;
		}else{

			MyStrcpy(msg.data,"Invalid login/password!\n");
			MsgSnd(TerminalOutPid,&msg);
			MsgRcv(my_pid,&msg);
		}
	}//inside whileloop 1

	  while(1){

      //prompt
	  proc_interface.flag = 1;
	  MyStrcpy(msg.data, "(MAARK) UserShell -> ");
	  MsgSnd(TerminalOutPid,&msg);
	  MsgRcv(my_pid,&msg);

      //get what's entered
	  MyStrcpy(something,msg.data);
	  MsgSnd(TerminalInPid,&msg);
	  MsgRcv(my_pid,&msg);

	  if(MyStrlen(msg.data) ==0){//skip if empty
		continue;
	  }
	  else if(MyStrcmp(msg.data, "out", sizeof("out"))||MyStrcmp(msg.data, "000", sizeof("000"))){
		break;//break to relogin
	  }
	  else if(MyStrcmp(msg.data, "dir", sizeof("dir"))||MyStrcmp(msg.data, "111", sizeof("111"))){
		Dir(msg.data, TerminalOutPid,FileSystemPid);
		continue;
	  }
	  else if(MyStrcmp(msg.data, "cat", MyStrlen("cat"))||MyStrcmp(msg.data, "222", MyStrlen("222"))){

		Cat(msg.data, TerminalOutPid, FileSystemPid);
        continue;
	  }
	  else{//COMMAND NOT MATCHING cat, dir, out
	    //check with file system
		 msg.code = CHK_OBJ;
		 MsgSnd(FileSystemPid,&msg);//should be 5
		 MsgRcv(my_pid,&msg);
		 ptr = (attr_t *)msg.data;
		 if(msg.code != GOOD || ptr->mode != MODE_EXEC){
				cons_printf("error message\n");
				 MyStrcpy(msg.data, "terminal: error message!\n");
				 MsgSnd(TerminalOutPid,&msg);
		         MsgRcv(my_pid,&msg);
				continue;
		 }
		 else{
			Fork((char*)ptr->data);

			//shell waits
			exit_num = GetTime();
			child_pid = Wait(&exit_num);
		 //prompt

		  MyStrcpy(msg.data, "child pid: ");
		  MsgSnd(TerminalOutPid,&msg);
		  MsgRcv(my_pid,&msg);
		  sprintf(something,"%d",child_pid);
		  MyStrcpy(msg.data, something);
		  MsgSnd(TerminalOutPid,&msg);
		  MsgRcv(my_pid,&msg);
		  sprintf(something,"%d",exit_num);
		  MyStrcpy(msg.data, " exit num: ");
		  MsgSnd(TerminalOutPid,&msg);
		  MsgRcv(my_pid,&msg);
		  MyStrcpy(msg.data, something);
		  MsgSnd(TerminalOutPid,&msg);
		  MsgRcv(my_pid,&msg);
		  MyStrcpy(msg.data, "\n");
		  MsgSnd(TerminalOutPid,&msg);
		  MsgRcv(my_pid,&msg);
		 }

	  }
   }//end forever loop

  }


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

void DirStr(attr_t *p, char *str) { // build str from attributes in given target
// msg.data has 2 parts: attr_t and target, p+1 points to target
   char *target = (char *)(p + 1);

// build str from attr_t p points to
   sprintf(str, " - - - -  size =%6d    %s\n", p->size, target);
   if ( A_ISDIR(p->mode) ) str[1] = 'd';         // mode is directory
   if ( QBIT_ON(p->mode, A_ROTH) ) str[3] = 'r'; // mode is readable
   if ( QBIT_ON(p->mode, A_WOTH) ) str[5] = 'w'; // mode is writable
   if ( QBIT_ON(p->mode, A_XOTH) ) str[7] = 'x'; // mode is executable
}

// "dir" command, UserShell talks to FileSystem and TerminalOut
// make sure cmd ends with \0: "dir\0" or "dir obj...\0"
void Dir(char *cmd, int TerminalOutPid, int FileSystemPid) {
   int my_pid;
   char str[101];
   msg_t msg;
   attr_t *p;

   my_pid = GetPid();

// if cmd is "ls" assume "ls /\0" (on root dir)
// else, assume user specified an target after first 3 letters "ls "
   if(cmd[3] == ' ') {
      cmd += 4; // skip 1st 4 letters "dir " and get the rest: obj...
   } else {
      cmd[0] = '/';
      cmd[1] = '\0'; // null-terminate the target
   }

// apply standard "check target" protocol
   msg.code = CHK_OBJ;
   MyStrcpy(msg.data, cmd);

   MsgSnd(FileSystemPid, &msg);     // send msg to FileSystem
   MsgRcv(my_pid, &msg);            // receive reply

   if(msg.code != GOOD) {           // chk result code
      MyStrcpy(msg.data, "Dir: CHK_OBJ returns NOT GOOD!\n\0");
      MsgSnd(TerminalOutPid, &msg);
      MsgRcv(my_pid, &msg);

      return;
   }

   p = (attr_t *)msg.data; // otherwise, code good, msg has "attr_t,"

   if(! A_ISDIR(p->mode)) {      // if it's file, "dir" it
      DirStr(p, str);             // str will be built and returned
      MyStrcpy(msg.data, str);   // go about since p pointed to msg.data
      MsgSnd(TerminalOutPid, &msg);
      MsgRcv(my_pid, &msg);

      return;
   }

// otherwise, it's a DIRECTORY! -- list each entry in it in loop.
// 1st request to open it, then issue reads in loop

// apply standard "open target" protocol
   msg.code = OPEN_OBJ;
   MyStrcpy(msg.data, cmd);
   MsgSnd(FileSystemPid, &msg);
   MsgRcv(my_pid, &msg);

   while(1) {                     // apply standard "read obj" protocol
      msg.code = READ_OBJ;
      MsgSnd(FileSystemPid, &msg);
      MsgRcv(my_pid, &msg);

      if(msg.code != GOOD) break; // EOF

// do same thing to show it via STANDOUT
      p = (attr_t *)msg.data;
      DirStr(p, str);                // str will be built and returned
      MyStrcpy(msg.data, str);
      MsgSnd(TerminalOutPid, &msg);  // show str onto terminal
      MsgRcv(my_pid, &msg);
   }

// apply "close obj" protocol with FileSystem
// if response is not good, display an error msg via TerminalOut...
   msg.code = CLOSE_OBJ;
   MsgSnd(FileSystemPid, &msg);
   MsgRcv(my_pid, &msg);

   if(msg.code != GOOD) {
      MyStrcpy(msg.data, "Dir: CLOSE_OBJ returns NOT GOOD!\n\0");
      MsgSnd(TerminalOutPid, &msg);
      MsgRcv(my_pid, &msg);

      return;
   }
}

// "cat" command, UserShell talks to FileSystem and TerminalOut
// make sure cmd ends with \0: "cat file\0"
void Cat(char *cmd, int TerminalOutPid, int FileSystemPid) {
   int my_pid;
   msg_t msg;
   attr_t *p;

   my_pid = GetPid();

   cmd += 4; // skip 1st 4 letters "cat " and get the rest

// apply standard "check target" protocol
   msg.code = CHK_OBJ;
   MyStrcpy(msg.data, cmd);

   MsgSnd(FileSystemPid, &msg); // send msg to FileSystem
   MsgRcv(my_pid, &msg);        // receive reply

   p = (attr_t *)msg.data;      // otherwise, code good, chk attr_t

   if(msg.code != GOOD || A_ISDIR(p->mode) ) { // if directory
      MyStrcpy(msg.data, "Usage: cat [path]filename\n\0");
      MsgSnd(TerminalOutPid, &msg);
      MsgRcv(my_pid, &msg);

      return;
   }

// 1st request to open it, then issue reads in loop

// apply standard "open obj" protocol
   msg.code = OPEN_OBJ;
   MyStrcpy(msg.data, cmd);
   MsgSnd(FileSystemPid, &msg);
   MsgRcv(my_pid, &msg);

   while(1) {      // apply standard "read target" protocol
      msg.code = READ_OBJ;
      MsgSnd(FileSystemPid, &msg);
      MsgRcv(my_pid, &msg);
// did it read OK?
      if(msg.code != GOOD) break; // until EOF or...
// otherwise, show file content via TerminalOut
      MsgSnd(TerminalOutPid, &msg);
      MsgRcv(my_pid, &msg);
   }
// apply standard "close target" protocol with FileSystem
// if response is not good, show error msg via TerminalOut...
   msg.code = CLOSE_OBJ;
   MsgSnd(FileSystemPid, &msg);
   MsgRcv(my_pid, &msg);

   if(msg.code != GOOD) {
      MyStrcpy(msg.data, "Cat: CLOSE_OBJ returns NOT GOOD!\n\0");
      MsgSnd(TerminalOutPid, &msg);
      MsgRcv(my_pid, &msg);

      return;
   }
}
