# hello.s
# syscall MsgSnd(mbox #, &msg) must have the the real msg RAM location.
# First subtract 4096 from ESP as the program runs to will get the starting
# RAM location. Since $msg # is virtual, given by link386, at 2G+X, subtract
# 2G from $msg (code below) will get X. Then add X to the starting location
# (= real address of the msg).

.text                       # code segment
.global _start              # _start is main()

_start:
   movl  $msg1, %eax         # msg is X bytes into this 4KB page
   subl  $0x80000000, %eax  # subtract 2G (virtual), get X (distance)

   pushl %esp               # esp starts atop of this RAM page
   popl  %ebx               # copy into ebx
   subl  $4096, %ebx        # minus 4KB --> start addr of process

   addl  %eax, %ebx         # add X to addr of page (real msg addr)
   pushl %ebx               # save/stack real msg addr
   pushl %ebx               # save/stack real msg addr

   movl  $4, %eax           # TerminalOutPid
   int   $54                # MsgSnd(4, &msg)

   int   $48                # eax = GetPid()
   popl  %ebx               # ebx = msg addr
   int   $55                # MsgRcv(my PID, &msg1)

   popl  %ebx               # get msg addr
   movl  4(%ebx), %eax

  #receive message
   movl  $msg1, %eax         # msg is X bytes into this 4KB page
   subl  $0x80000000, %eax  # subtract 2G (virtual), get X (distance)

   pushl %esp               # esp starts atop of this RAM page
   popl  %ebx               # copy into ebx
   subl  $4096, %ebx        # minus 4KB --> start addr of process

   addl  %eax, %ebx         # add X to addr of page (real msg addr)
   pushl %ebx               # save/stack real msg addr
   pushl %ebx               # save/stack real msg addr

   movl  $3, %eax           # TerminalInPid
   int   $54                # MsgSnd(3, &msg2)

   int   $48                # eax = GetPid()
   popl  %ebx               # ebx = msg addr
   int   $55                # MsgRcv(my PID, &msg)

   popl  %ebx               # get msg addr
   movl  4(%ebx), %eax

   #return after got what was entered
   movl  $msg3, %eax         # msg is X bytes into this 4KB page
   subl  $0x80000000, %eax  # subtract 2G (virtual), get X (distance)

   pushl %esp               # esp starts atop of this RAM page
   popl  %ebx               # copy into ebx
   subl  $4096, %ebx        # minus 4KB --> start addr of process

   addl  %eax, %ebx         # add X to addr of page (real msg addr)
   pushl %ebx               # save/stack real msg addr
   pushl %ebx               # save/stack real msg addr

   movl  $4, %eax           # TerminalOutnPid
   int   $54                # MsgSnd(3, &msg2)

   int   $48                # eax = GetPid()
   popl  %ebx               # ebx = msg addr
   int   $55                # MsgRcv(my PID, &msg)

   popl  %ebx               # get msg addr
   movl  4(%ebx), %eax

   #print out what to enter
   movl  $msg2, %eax         # msg is X bytes into this 4KB page
   subl  $0x80000000, %eax  # subtract 2G (virtual), get X (distance)

   pushl %esp               # esp starts atop of this RAM page
   popl  %ebx               # copy into ebx
   subl  $4096, %ebx        # minus 4KB --> start addr of process

   addl  %eax, %ebx         # add X to addr of page (real msg addr)
   pushl %ebx               # save/stack real msg addr
   pushl %ebx               # save/stack real msg addr

   movl  $4, %eax           # TerminalOutnPid
   int   $54                # MsgSnd(3, &msg2)

   int   $48                # eax = GetPid()
   popl  %ebx               # ebx = msg addr
   int   $55                # MsgRcv(my PID, &msg)

   popl  %ebx               # get msg addr
   movl  4(%ebx), %eax



   #get what was entered
   movl  $msg1, %eax         # msg is X bytes into this 4KB page
   subl  $0x80000000, %eax  # subtract 2G (virtual), get X (distance)

   pushl %esp               # esp starts atop of this RAM page
   popl  %ebx               # copy into ebx
   subl  $4096, %ebx        # minus 4KB --> start addr of process

   addl  %eax, %ebx         # add X to addr of page (real msg addr)
   pushl %ebx               # save/stack real msg addr
   pushl %ebx               # save/stack real msg addr

   movl  $4, %eax           # TerminalOutnPid
   int   $54                # MsgSnd(3, &msg2)

   int   $48                # eax = GetPid()
   popl  %ebx               # ebx = msg addr
   int   $55                # MsgRcv(my PID, &msg)

   popl  %ebx               # get msg addr
   movl  4(%ebx), %eax

   movl  $msg3, %eax         # msg is X bytes into this 4KB page
   subl  $0x80000000, %eax  # subtract 2G (virtual), get X (distance)

   pushl %esp               # esp starts atop of this RAM page
   popl  %ebx               # copy into ebx
   subl  $4096, %ebx        # minus 4KB --> start addr of process

   addl  %eax, %ebx         # add X to addr of page (real msg addr)
   pushl %ebx               # save/stack real msg addr
   pushl %ebx               # save/stack real msg addr

   movl  $4, %eax           # TerminalOutnPid
   int   $54                # MsgSnd(3, &msg2)

   int   $48                # eax = GetPid()
   popl  %ebx               # ebx = msg addr
   int   $55                # MsgRcv(my PID, &msg)

   popl  %ebx               # get msg addr
   movl  4(%ebx), %eax




   int   $58                # Exit(time_stamp)

.data                       # data segment always follows code segment in RAM
msg1:                        # msg
   .long 0                  # msg.sender
   .long 0                  # msg.time_stamp
   .ascii "Enter something-> " # data (35 chars) # msg.data
   .rept 66                 # rest
      .ascii "\0"           # with null characters
   .endr
   .long 0                  # msg.code
   .long 0                  # msg.number[0]
   .long 0                  # msg.number[1]
   .long 0                  # msg.number[2]
msg2:                        # msg
   .long 0                  # msg.sender
   .long 0                  # msg.time_stamp
   .ascii "You entered-> " # data (35 chars) # msg.data
   .rept 66                 # rest
      .ascii "\0"           # with null characters
   .endr
   .long 0                  # msg.code
   .long 0                  # msg.number[0]
   .long 0                  # msg.number[1]
   .long 0                  # msg.number[2]
msg3:                        # msg
   .long 0                  # msg.sender
   .long 0                  # msg.time_stamp
   .ascii "\n" # data (35 chars) # msg.data
   .rept 66                 # rest
      .ascii "\0"           # with null characters
   .endr
   .long 0                  # msg.code
   .long 0                  # msg.number[0]
   .long 0                  # msg.number[1]
   .long 0                  # msg.number[2]

 ##########
  #//(send msg to TerminalOutPid, receive its reply)
	#  MyStrcpy(msg.data,"\nEnter something->");
	 # MsgSnd(TerminalOutPid,&msg);
	  #MsgRcv(my_pid,&msg);

      #//get what's entered
	  #MyStrcpy(something,msg.data);
	  #MsgSnd(TerminalInPid,&msg);
	  #MsgRcv(my_pid,&msg);

	  #MyStrcpy(something,msg.data);

	  #MyStrcpy(msg.data,"You've entered ->");
	  #MsgSnd(TerminalOutPid,&msg);
	  #MsgRcv(my_pid,&msg);

	  #//(send msg to TerminalOutPid, receive its reply)
	  #MyStrcpy(msg.data,something);
	  #MsgSnd(TerminalOutPid,&msg);
	  #MsgRcv(my_pid,&msg);
