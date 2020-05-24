# User Mode Thread Library
## Main Idea
Understand how threads work and try implementing independent and parallel executions in a process

## Description
This is a project trying to implement a thread system for Linux. This is done by using 
the functions:
pthread_ create() - Creates a new thread within a process
pthread_self() - return the thread ID
pthread_exit() - terminates the thread

For pthread_create, I set a flag that showed if main()'s thread was initialized. If not, I initialized it and set the timer using a helper function,thereby increasing the "runners variable" showing the number of active threads( this increments everytime pthread_create is called. After doing so, I manually set start routine. currentThread is what is used for schedule as well as context switching. 

For pthread_self, I just returned threads[currentThread].thread_id which
is runners stored in the TCB of that speciic thread.

For pthread_exit,I made status of the thread to EXIT and decreased runners since it is no longer executing. In doing so,
I promptly call schedule() here. There are two places I put schedule (pthreads_create and pthreads_exit). This is so I can schedule whatever thread created and
schedule the next thread after the previous thread ends.

The way my round robin works is it looks for status READY in the TCB
and if that hasn't set to RUN or EXIT, it will choose this thread to long jump using currentThread to use as index.

## Problem 
One unsuccessful thing was trying to context switch. For some reason, the timer
would work and longjmp wouldn't work and vice versa.GDB was hard to do 
because for some reason longjmp would call millions of instructions and i 
would have to step each time since the registers always changes.


