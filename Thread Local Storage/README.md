# Thread Local Storage
## Idea
Trying to work with memory managements and provide memory regions for threads

## Description
This is to try and implement memory regions for threads.
To begin, I do not know hash maps (or atlest I don't remember how to implement it) therefore I used a linked list for mapping of the threads to TLS. 

tls_protect and tls_unprotect are helped functions that essentially help protect and unprotect the pages. It was given to us in discussion slides and it uses mprotect.

mprotect bascially gives a SIGSEGV signal when a calling process tries to access a manner in which specificed by its arguments, *addr,len,and prot. In doing so, this essentially protects a region of memory.

On the other hand, tls_unprotect allows access to the region.

tls_init is to initialize the signals and signal flgas SIGBUS and SIGSEGV and other signal handling. This only runs once and has a global flag.

tls_handle_page_fault is our signal handler for when SIGSEGV is invoked. it first checks for LSA for the thread and if their is none, it sneds a signal otherwise pthread_exit is called and there is a segmentation fault. When the signal is caught, we simply terminate the running thread.This is crucial in distinguishing between cases in which an LSA is involved or not involved.

tls_create first checks if tls_init is called. If not, it is called there. Otherwise it error checks to see if size is less than 0 and returns -1 if so;
Now we error check to see if there is already an LSA for the current thread.
If all the error checks are passed, calloc is used to allocate TLS and its new data is entered. After initialzing TLS pages using array of pointers with calloc, we allocate all pages for this TLS given to us through discussion. Then we add it to the tail of the linked list.

tls_destroy essentially does the same error checks as create besides the size.Simply put, it cleans up all pages and cleans up TLS by using munmap which delets the mappings for the specified address. At the end it removes the mapping from the linked list and gets rid of it.

tls_write does the same error checks which is the LSA. However if offset+length is greater than the size of tls it returns -1. If it passes the error checks, we unprotect the pages for the thread and perform the write operation. Copy on Write is used (im still trying to figure abit of it out as I don't think it's working correctly) to write into the pages. After doing so, we protect the pages.

tls_read does the samee error handling and unprotects the pages. it is almost similar to the write function except there are a bit of adjustments which is given to us from the discussion slides.

tls_clone does a different error handling in whihc it checks both the current and target thread for an LSA.
After doing it clones, allocates,TLS,and copies the pages. Then it puts it at the end of the tail of the global link list.

The hard part was using stdint.h. Just because of this I had a hard time trying to figure out whats wrong. I also kept using malloc in different places before realizing alot of the error handling is the same.
