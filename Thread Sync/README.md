# Thread Synchronization
## Idea
Trying to exted threading library with synchronization primitives so threads can interact

## Description
I tried to put a lockign mechanisms to turn on/off thread interactivity, join threads and add semaphore support.
### New Things from User Thread Library Project

lock() -> can no longer be interrupeted. Uses sigprogmask.
unlock() -> thread resumes just like normal in the Next SIGALRM
pthread_join -> waits until target thread terminates for currentThread resumes. 

sem_init -> initialized semaphore, uses a "queue" with head and tail pointer
uses __align to store semaphore values and index
sem_wait -> useses __align and decrements current_value to block currentThread
increases the number of blocked threads and moves the tail one and sets
the head to the thread id
sem_post -> uses __align and increments semaphore current_value which unlocks the semaphore
sem_destory -> destorys semaphore depending on *sem argument

Trouble with different start_routines. Looks like it works but tests wont't passdue to this. I think producer-consumer test wont work
8,9,10 sometihng in pthread_join or pthread_exit is not working correctly
