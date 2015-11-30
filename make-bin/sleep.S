# sleep.s
#
# below as a demo code, it sleeps for 3 seconds, change it to:
#    1. sleep for period of seconds matching its PID
#    2. exits with a number that is the system time
#
# To compile and extract the executable, run shell script "make-bin sleep"
# (DO NOT MISTYPE with sleep.s which will DELETE your sleep.s code!)
# It does:
#    make-bin.pl sleep.s, which includes:
#       as --32 sleep.s -o sleep.o 
#       link386 -nostartfiles -userapp sleep.o -o sleep
# sleep.bin is the final file to be included into the file system

# If curious about how code in memory: as -a sleep.s

.text               # code segment 
.global _start      # _start is gonna be main()

_start:             # instructions starts
   int  $48				# seconds to sleep, movl $3, %eax 
   int  $50         # call sleep service

   int  $49				# exit number, movl $77, %eax
   int  $58         # call exit service

.data               # examples of defining data if needed
x:
   .long 1          # an integer, 100 initially
str1:
   .ascii "this is a string\n\0" # a null-terminated string
str2:                     # another string, 101 chars total
   .ascii "Greetings! Team MAARK\n"
   .rept 100              # repeat: 100 times
      .ascii "\0"         # NUL characters
   .endr                  # end of repeat