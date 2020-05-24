#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <setjmp.h>
#include <semaphore.h>
#include <sys/mman.h>
#include "tls.h"
#include <stdint.h>
#include <string.h>

struct page{
        uintptr_t address;
        int ref_count;
};

int page_size;

typedef struct thread_local_storage{
        pthread_t tid;
        unsigned int size;
        //uintptr_t size;
        //uintptr_t page_num;
        unsigned int page_num;
        struct page **pages;
} TLS;

typedef struct link{
        struct link *after;
        struct link *before;
        TLS *tls;
}link_list;

link_list *head;
link_list *tail;
int LSA;
int idx;
int fault;
pthread_t id;
int idx,cnt;
char *src;
void tls_protect(struct page *p);
void tls_unprotect(struct page *p);
void tls_init();
void tls_handle_page_fault(int sig,siginfo_t *si,void *context);

int init_flag = 0;
void tls_init(){
        struct sigaction sigact;
        page_size = getpagesize();
        sigemptyset(&sigact.sa_mask);
        sigact.sa_flags = (SA_SIGINFO);
        sigact.sa_sigaction = tls_handle_page_fault;
        sigaction(SIGBUS, &sigact, NULL);
        sigaction(SIGSEGV, &sigact, NULL);
        init_flag = 1;
	link_list *init = malloc(sizeof(link_list));
        init->after = NULL;
        init->before = NULL;
        head = init;
        tail = init;

        printf("we're in init\n");
}

void tls_handle_page_fault(int sig,siginfo_t *si, void *context){


        unsigned int p_fault = ((uintptr_t) si -> si_addr) & ~(page_size -1);
        LSA = 0;
        fault = 0;
        int index;

        link_list *search = head;
        while(search->after != NULL){
                search = search->after;
                int index = search -> tls -> page_num;

                int i;
                for (i=0;i<index;i++){
                        if(search->tls->pages[index]->address=p_fault){
                                LSA = 0;
                                pthread_exit(NULL);
                        }
                       else
                                LSA =1;
                }

                if(!LSA){
                        signal(SIGSEGV, SIG_DFL);
                        signal(SIGBUS, SIG_DFL);
                        raise(sig);
                }
                else{
                        int j;
                        for(j =0; j<search->tls->page_num+1; j++){
                                if(search->tls->pages[j]->address=p_fault){
                                        pthread_exit(NULL);
                                        fault = 1;
                                }
                        }

                        if(!fault){

                        signal(SIGSEGV, SIG_DFL);
                        signal(SIGBUS, SIG_DFL);
                        raise(sig);
                        }
                }
        }
}

void tls_unprotect(struct page *p){
        printf("unprotect\n");
        if(mprotect((void *)(intptr_t)p->address, page_size,PROT_READ | PROT_WRITE)){
                fprintf(stderr,"tls_unprotect: coudn't unprotect page\n");
                exit(1);
        }
}

void tls_protect(struct page *p){
        printf("protect\n");
        if(mprotect((void *)(intptr_t) p->address, page_size, 0)){
                fprintf(stderr, "tls_protect: coundn't protect page\n");
                exit(1);
        }
}

int tls_create (unsigned int size){

        if(!init_flag){
                tls_init();
                //init_flag = 1;
        }
        else if(size<0){
                printf("\n error:size is negative..IMPOSSIBLE!\n");
                return -1;
        }

        id = pthread_self();
        link_list *search = head;

        while (search->after != NULL){
                search = search->after;
                if (search->tls->tid == id){
                        printf("\n error: no LSA for poor thread\n");
                        return -1;
                }
        }

        TLS *new_TLS = (TLS*)calloc(1,sizeof(TLS));
	//don't use malloc gdmi
       // new_TLS->pages= (struct page**)malloc(sizeof(struct page*)*new_TLS->page_num);
        new_TLS->page_num = (size-1)/page_size+1;
        new_TLS->size = size;
        new_TLS->tid = id;
	new_TLS->pages = (struct page **)calloc(new_TLS->page_num,sizeof(struct page*));

        int i;
        for (i = 0; i< new_TLS->page_num; i++){
                struct page *p;
                p = (struct page*)calloc(1,sizeof(struct page));
                p->ref_count = 1;
                p->address = (uintptr_t)mmap(0, page_size, 0, MAP_ANON | MAP_PRIVATE,0 ,0); //try the unistd_32 thingy
                new_TLS->pages[i] = p;
        }
        tail->after = malloc(sizeof(link_list));

        tail->after->after = NULL;
	link_list *link = tail->after;
        tail->after->tls = new_TLS;
        tail->after->before = tail;
        tail = link;

        return 0;
}

int tls_destroy(){
        if(!init_flag)
                return -1;

        id = pthread_self();
        link_list *search = head;

        while(search->after != NULL){
                search = search->after;

                if(search->after ==NULL && search->tls->tid != id){
                        printf("\n error:no LSA\n");
                        return -1;
                }
                else if(search->tls->tid == id)
                        break;
        }

        int index = search->tls->page_num;
        int i;
        for(i = 0; i< index; i++){
                if(search->tls->pages[i]->ref_count != 1)
                        search->tls->pages[i]->ref_count--;
                else if(search->tls->pages[i]->ref_count ==1){
                        munmap((void*)(intptr_t)search->tls->pages[i]->address,page_size);;
			}
        }

        search->before->after= search->after;
        if(search->after == NULL)
                tail = search->before;
        else
                search->after->before = search->before;

        free(search->tls->pages);
        free(search->tls);
        free(search);
        return 0;
}

int tls_write(unsigned int offset, unsigned int length, char *buffer){

        if(!init_flag){
                return -1;
        }

        id = pthread_self();
        link_list *search = head;

        while(search->after != NULL){
                search = search->after;

                if(search->after ==NULL && search->tls->tid != id){
                        printf("\n error:no LSA\n");
                        return -1;
                }
                else if(search->tls->tid == id)
                        break;
        }

        unsigned int simple = offset + length;
        if( simple > search->tls->size){
                return -1;
        }

        int index = search->tls->page_num;
        int i;
        for( i = 0; i <index; i++)
                tls_unprotect(search->tls->pages[i]);
	
        for (cnt = 0, idx = offset; idx< simple; ++cnt, ++idx){
                struct page *p, *copy;
                unsigned int pn,poff;
                pn = idx/page_size;
                poff = idx%page_size;
                p = search->tls->pages[pn];

                if(p->ref_count > 1){
                        copy = (struct page *) calloc(1, sizeof(struct page));
                        copy->ref_count = 1;
                        copy->address = (uintptr_t)mmap(0,page_size, PROT_WRITE, MAP_ANON | MAP_PRIVATE, 0,0);
                        search->tls->pages[pn] = copy;
                        //come back later; this one might be useless after looking at piazza use mem cpy 

                       // for(i = 0; i< index; i++){
                         //       char *copy_addy = ((uint8_t*)p->address) + i;
                           //     char *addy_copied = ((uint8_t*)copy->address)+i;
                             //   *copy_addy = *addy_copied;
                        //}
		memcpy((uint8_t*)copy->address,(uint8_t*)p->address,page_size);
                p->ref_count--;
                p = copy;
                tls_protect(p);
                }
        char* dst = ((uint8_t*) p->address) + poff;
        *dst = buffer[cnt];
        }

        for(i = 0; i<index; i++)
                tls_protect(search->tls->pages[i]);

return 0;
}

int tls_read(unsigned int offset, unsigned int length, char *buffer){

  if(!init_flag){
                return -1;
        }

        id = pthread_self();
        link_list *search = head;

        while(search->after != NULL){
                search = search->after;

                if(search->after ==NULL && search->tls->tid != id){
                        printf("\n error:no LSA\n");
                        return -1;
                }
                else if(search->tls->tid == id)
                        break;
        }

        unsigned int simple = offset + length;
        if( simple > search->tls->size){
                return -1;
        }

        int index = search->tls->page_num;
        int i;
        for( i = 0; i <index; i++)
                tls_unprotect(search->tls->pages[i]);
	
        for (cnt = 0, idx = offset; idx< simple; ++cnt, ++idx){
                struct page *p;
                unsigned int pn,poff;
                pn = idx/page_size;
                poff = idx%page_size;
                p = search->tls->pages[pn];
                src = ((uint8_t*)p->address) + poff;
                buffer[cnt] = *src;
        }

        for(i = 0; i<index; i++)
                tls_protect(search->tls->pages[i]);

return 0;
}

int tls_clone(pthread_t tid){
          if(!init_flag){
                return -1;
        }

        id = pthread_self();
        link_list *search = head;
        link_list *check = head;

        while(search->after != NULL){
                search = search->after;

                if(search->tls->tid == id){
                        printf("\n error: LSA exists \n");
                        return -1;
                }
        }

        while(check->after !=NULL){
                check = check->after;
                 if(check->tls->tid == tid)
                        break;
		else if(check -> after == NULL && check->tls->tid!= tid)
                        return -1;

        }
	
        printf("making dittos to make some mewtwos\n");
        TLS* ditto = (TLS *)calloc(1,sizeof(TLS));
        //ditto->pages = (struct page **)malloc(sizeof(struct page*) * ditto->page_num);
        ditto->size = check->tls->size;
        ditto->page_num = check->tls->page_num;
	ditto->pages = (struct page **)calloc(ditto->page_num, sizeof(struct page*));
        ditto->tid = id;

        int i;
        for(i = 0; i< ditto->page_num; i++){
                ditto->pages[i]->ref_count++;
                ditto->pages[i] = check->tls->pages[i];
        }

        tail->after = malloc(sizeof(link_list));
        tail->after->after = NULL;
	link_list *link = tail->after;
        tail->after->before = tail;
        tail->after->tls = ditto;
        tail = link;
        return 0;
}

