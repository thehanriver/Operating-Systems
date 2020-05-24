#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>

#define THREAD_CNT 3

void *count(void *arg){

	unsigned long int c = (unsigned long int)arg;
	int i;
	for (i=0;i<c;i++){
		if((i%1000) == 0){ 
			printf("tid: 0x%x Just counted to %d of %ld\n", \
			(unsigned int)pthread_self(),i,c);
		}
	}
	return arg;
}

int main(int argc, char **argv){
	pthread_t threads[THREAD_CNT];
	int i;
	unsigned long int cnt = 1000;

	//create THREAD_CNT threads
	for(i=0; i<THREAD_CNT; i++){
		pthread_create(&threads[i],NULL,count,(void *)((i+1)*cnt));
	}

	count((void*)(cnt*(THREAD_CNT + 1)));
	return 0;
}
