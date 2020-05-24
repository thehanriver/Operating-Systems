#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "ec440threads.h"
#include <setjmp.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>


#define BLOCK 3
#define READY 0
#define MADE 2
#define EXIT 1
#define JB_RBX  0
#define JB_RBP  1
#define JB_R12  2
#define JB_R13  3
#define JB_R14  4
#define JB_R15  5
#define JB_RSP  6
#define JB_PC   7

// defining max threads,max byte for stack, and 50 ms freq for alaram
#define MAX_THREADS 128
#define MAX_SIZE 32767
#define FREQ 50

static int runners=0;
static int currentThread=0;
static int main_man = 0 ;
static int created[MAX_THREADS] = {0};
static int new = 0;

void schedule();
void helper();
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(start_routine) (void*), void *arg);
pthread_t pthread_self(void);
void pthread_exit(void *retval);
void unlock();
void lock();
int pthread_join(pthread_t thread, void **value_ptr);

struct thread {
        pthread_t thread_id;
        jmp_buf BUFF;
        unsigned int *stackem;
        unsigned long int status;
        pthread_t join[MAX_THREADS];
        void *ex;
        };

struct thread threads[MAX_THREADS];

void schedule(){

        // if setjump is good and doesnt return something from !=0
        // finds next thread that is ready using round robin

if(setjmp(threads[currentThread].BUFF) == 0){
        lock();
        //if threads are greater than 128 or there are more queued threads
        //than threads that are ready, loops back to 0
        printf("runners: %d\n", runners);
    if(currentThread>MAX_THREADS || currentThread >= runners-1){
                currentThread = 0;

                printf("RESET CT to 0\n");
    }
    //otherwise look at next ready to run thread
    else{
        currentThread++;
        printf("increment CT: %d\n",currentThread);
        }

        //looks from currentThread 0 - 127 to find status ready
     while(threads[currentThread].status!=READY){

        currentThread = (currentThread +1 ) % MAX_THREADS;
        printf("inside the while loop, CT is:  %d\n", currentThread);
        }
        unlock();
        printf("about to jump to : %d\n", currentThread);
        longjmp(threads[currentThread].BUFF,1);

//error check to see if setjump isn't working properly
}else{

        unlock();}
}

//initialze "main" thread
void helper() {
lock();
// main thread is the first thread and save buffer 0 as main threaad
    threads[0].thread_id=0;
    threads[0].status= READY;
    threads[0].stackem = NULL;
    setjmp(threads[0].BUFF);
    main_man=1;



//settup for sig alaram and signal handler to make it point to schedule()
//whenever time is up
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = &schedule;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_NODEFER;
    sigaction(SIGALRM, &act, NULL);

//settup the timer for the freq so SIGALRM goes off every 50 ms
    struct itimerval tik;
    tik.it_interval.tv_usec = FREQ*1000;
    tik.it_interval.tv_sec = 0;
    tik.it_value.tv_usec = FREQ* 1000;
    tik.it_value.tv_sec = 0;

    setitimer (ITIMER_REAL ,&tik, NULL);
    runners++;
    printf("we got main\n");
unlock();
}


int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(start_routine) (void*), void *arg){
lock();
         //flag for main just to initialize it once
         if (main_man == 0){
            helper();
        }


        if(runners <128){
        int i;
        for ( i = 1; i < MAX_THREADS; i++){
                if (created[i] == 0){
                        new = i;
                        break;
                }
        }

        threads[new].thread_id=new;
        threads[new].status= READY;



        threads[new].stackem = malloc(MAX_SIZE) + MAX_SIZE - 4;
        *(threads[new].stackem) = (unsigned long int) &pthread_exit;

        //set buffer for active thread empty state
        setjmp(threads[new].BUFF);
        //printf("just made thread %d \n", runners);


        //now manually setup start routine and add to jump buff
        threads[new].BUFF[0].__jmpbuf[JB_PC]=ptr_mangle((unsigned long int)start_thunk);
        threads[new].BUFF[0].__jmpbuf[JB_RSP]=ptr_mangle((unsigned long int)threads[new].stackem);
        threads[new].BUFF[0].__jmpbuf[JB_R13]=(unsigned long int)arg;
        threads[new].BUFF[0].__jmpbuf[JB_R12]=(unsigned long int)start_routine;

        }
        //if more active threads than MAX threads then print the error
        else if (runners >128){
        printf("ERROR\n");
        exit(1);
        }
        //printf("we're in pthread create and new is %d\n",new);
        //increment the number of active threads and schedule it
        runners++;
        created[new] = 1;
        printf("just made thread %d \n", runners);
        unlock();
        schedule();
        return 0;
}

//just returns the thread id
pthread_t pthread_self(){
        return threads[currentThread].thread_id;
}


void pthread_exit(void *retval){
        lock();
        created[currentThread] = 0;
        threads[currentThread].status = EXIT;


        int i = 0;
        int clearing = 0;
        for (i=0; i<MAX_THREADS; i ++){
                clearing = threads[currentThread].join[i];
                threads[clearing].status = READY;
        }
        runners--;
        unlock();
        schedule();
        __builtin_unreachable();

}

void lock(){

sigset_t signal_set;
sigemptyset(&signal_set);
sigaddset(&signal_set, SIGALRM);
sigprocmask(SIG_BLOCK, &signal_set, NULL);
}

void unlock(){

sigset_t signal_set;
sigemptyset(&signal_set);
sigaddset(&signal_set, SIGALRM);
sigprocmask(SIG_UNBLOCK, &signal_set, NULL);
}

int pthread_join(pthread_t thread, void **value_ptr){
lock();
if( value_ptr !=NULL){
*value_ptr = threads[thread].ex;
}

int find = 0;
int find_thread=0;
int find_join=0;

while ( find < MAX_THREADS && threads[find].thread_id != thread){

        find++;
}

if(threads[find].status != EXIT){
        threads[currentThread].status = BLOCK;
        for ( find_thread = 0; find_thread< MAX_THREADS; find_thread++){
                if (threads[find_thread].thread_id == thread){
                        for(find_join=0; find_join < MAX_THREADS; find_join++){
                                if(threads[find_thread].join[find_join] ==0){
                                        threads[find_thread].join[find_join] = threads[currentThread].thread_id;
break;
                                }
                        }
                }
        }
}
schedule();


unlock();
return 0;
}

