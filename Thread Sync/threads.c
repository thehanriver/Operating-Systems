#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "ec440threads.h"
#include <setjmp.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

#define BLOCK 3
#define MADE 0
#define EXIT 2
#define READY 1
#define JB_RBX  0
#define JB_RBP  1
#define JB_R12  2
#define JB_R13  3
#define JB_R14  4
#define JB_R15  5
#define JB_RSP  6
#define JB_PC   7

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

//project3
void unlock();
void lock();
int pthread_join(pthread_t thread, void **value_ptr);
void pthread_exit_wrapper();
int sem_init(sem_t *sem, int pshared, unsigned value);
int sem_wait(sem_t *sem);
int sem_post(sem_t *sem);
int sem_destroy(sem_t *sem);

struct thread {
        pthread_t thread_id;
        jmp_buf BUFF;
        unsigned long int *stackem;
        unsigned int status;
        unsigned int prev_stat;
        pthread_t join_stat;
        void *ex;
        };

struct thread threads[MAX_THREADS];

struct semaphore{
        unsigned int current_value;
        unsigned int c_threads;
        //marker for beginning of queue
        unsigned int front;
        //marker for end of queue
        unsigned int end;
        //0 not used 1 used
        unsigned int flag;
        //queue for sem
        unsigned int queue[MAX_THREADS];
        sem_t *id;
};

static struct semaphore semaphores[MAX_THREADS];

int sem_init(sem_t *sem, int pshared, unsigned value){
        lock();

        int i;
        //trying to find semaphore that hasn't been used
        while( i<MAX_THREADS){
		i++;
		if(semaphores[i].flag == 0)
		break;
		
	}

        sem->__align = i;
        //save for other sems

        semaphores[sem->__align].current_value = value;

        //mark used and create fake head and tail pointer
        semaphores[sem->__align].front=0;
        semaphores[sem->__align].end=0;
        semaphores[sem->__align].flag=1;
        semaphores[sem->__align].id = sem;


        unlock();
        return 0;
}

int sem_wait(sem_t *sem){
        lock();
        //note to mario: SEM NEVER FALLS BELOW ZEROOOOOOOOO
        if(semaphores[sem->__align].current_value>0){
                semaphores[sem->__align].current_value--;

                unlock();
                return 0;
        }
        if(semaphores[sem->__align].queue[semaphores[sem->__align].front] ==0){
                //add currentThreads id to blocked queue

                threads[currentThread].status = BLOCK;
                semaphores[sem->__align].queue[semaphores[sem->__align].front] = threads[currentThread].thread_id;
                semaphores[sem->__align].c_threads++;
                semaphores[sem->__align].end++;
                //increment blocked threads in queue threads and move tail pointerup
        }
        else{
        unlock();
        return -1;
        }

        unlock();
        return 0;
}

int sem_post(sem_t *sem){
        lock();
        //increments sem
        if(semaphores[sem->__align].id == sem){
		 if( semaphores[sem->__align].c_threads != 0){
               		 semaphores[sem->__align].current_value++;}
        }
        unlock();
        return 0;
}

int sem_destroy(sem_t *sem){
        lock();
        if(semaphores[sem->__align].id == sem){
		if( semaphores[sem->__align].flag == 0){
                	semaphores[sem->__align].c_threads = 0;}
                unlock();
                return 0;
        }
        unlock();
        return 0;
}

void schedule(){


        if(setjmp(threads[currentThread].BUFF) == 0){
                printf("runners: %d\n", runners);
                lock();
                if(currentThread>MAX_THREADS || currentThread >= runners-1){
                        currentThread = 0;
                        //printf("RESET CT to 0\n");
                }

                else{
                        currentThread++;
                        //printf("increment CT: %d\n",currentThread);
                }
                //printf("currentThread, %d , is in state : %lu\n",currentThread,threads[currentThread].status);

                while(threads[currentThread].status!=READY){
                        currentThread = (currentThread +1 ) % MAX_THREADS;
                        //printf("inside the while loop, CT is:  %d\n", currentThread);
                }

                unlock();
                printf("about to jump to : %d\n", currentThread);
                longjmp(threads[currentThread].BUFF,1);

        }else
                unlock();
}

void helper() {

        threads[0].ex = NULL;
        threads[0].thread_id=0;
        threads[0].status= READY;
        threads[0].stackem = NULL;
        main_man=1;

        struct sigaction act;
        memset(&act, 0, sizeof(act));
        act.sa_sigaction = &schedule;
        sigemptyset(&act.sa_mask);
        act.sa_flags = SA_NODEFER;
        sigaction(SIGALRM, &act, NULL);

        struct itimerval tik;
        tik.it_interval.tv_usec = FREQ*1000;
        tik.it_interval.tv_sec = 0;
        tik.it_value.tv_usec = FREQ* 1000;
        tik.it_value.tv_sec = 0;
        setitimer (ITIMER_REAL ,&tik, NULL);

        created[0] = 1;
        runners++;

        printf("we got main\n");
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
        threads[new].prev_stat = MADE;
        threads[new].ex = NULL;
        threads[new].join_stat = -1;
	*thread = new;
        threads[new].stackem = malloc(MAX_SIZE) + MAX_SIZE - 4;
        *(threads[new].stackem) = (unsigned long int)&pthread_exit_wrapper;

        //set buffer for active thread empty state
        setjmp(threads[new].BUFF);

        //now manually setup start routine and add to jump buff
        threads[new].BUFF[0].__jmpbuf[JB_PC]=ptr_mangle((unsigned long int)start_thunk);
        threads[new].BUFF[0].__jmpbuf[JB_RSP]=ptr_mangle((unsigned long int)threads[new].stackem);
        threads[new].BUFF[0].__jmpbuf[JB_R13]=(unsigned long int)arg;
        threads[new].BUFF[0].__jmpbuf[JB_R12]=(unsigned long int)start_routine;
        }

        //if more active threads than MAX threads then print the error
        else if (runners >128||runners==128){
        printf("ERROR\n");
	exit(-1);

        }

        //increment the number of active threads and schedule it
        runners++;
        created[new] = 1;
        unlock();
        schedule();
        return 0;
}

pthread_t pthread_self(){
        return threads[currentThread].thread_id;
}

void pthread_exit_wrapper(){
        unsigned long int res;
        asm("movq %%rax, %0\n":"=r"(res));
        pthread_exit((void *) res);
}

void pthread_exit(void *retval){
        lock();
        threads[currentThread].status = EXIT;
        threads[currentThread].ex = retval;
        //free(threads[currentThread].stackem);
        printf("im in ext\n");
        //int i;
        //int clearit;
        //for(i=0;i<MAX_THREADS;i++){
                //clearit = threads[currentThread].join_stat;
                //threads[clearit].status = READY;
        //}
        if (threads[currentThread].join_stat != -1){
                if(threads[threads[currentThread].join_stat].status == BLOCK){
                        threads[threads[currentThread].join_stat].status = READY;
                }
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
        
        if(value_ptr == NULL && threads[thread].prev_stat != MADE && threads[thread].status != EXIT && threads[thread].join_stat == -1 ){
                threads[currentThread].prev_stat = threads[currentThread].status;
                threads[currentThread].status = BLOCK;
		printf("blocking in pthread_join\n");
                threads[thread].join_stat = threads[currentThread].thread_id;
                schedule();
                unlock();
                return 0;

        }else if(threads[thread].join_stat !=-1){
        unlock();
        return -1;}

        else{
        unlock();
        return 0;}
}

