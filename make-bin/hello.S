# hello.s
# syscall MsgSnd(mbox #, &msg) must have the the real msg RAM location.
# First subtract 4096 from ESP as the program runs to will get the starting
# RAM location. Since $msg # is virtual, given by link386, at 2G+X, subtract
# 2G from $msg (code below) will get X. Then add X to the starting location
# (= real address of the msg).

.text                       # code segment
.global _start              # _start is main()

_start:
   movl  $msg, %eax         # msg is X bytes into this 4KB page
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
   int   $55                # MsgRcv(my PID, &msg)

   popl  %ebx               # get msg addr
   movl  4(%ebx), %eax
   int   $58                # Exit(time_stamp)

.data                       # data segment always follows code segment in RAM
msg:                        # msg
   .long 0                  # msg.sender
   .long 0                  # msg.time_stamp
   .ascii "Team MAARK, How're you doing?\n" # data (35 chars) # msg.data
   .rept 66                 # rest
      .ascii "\0"           # with null characters
   .endr
   .long 0                  # msg.code
   .long 0                  # msg.number[0]
   .long 0                  # msg.number[1]
   .long 0                  # msg.number[2]
