#include "shell.h"

//global variables
char readit[BUFFER];
char *arguments[BUFFER];
char *commands[BUFFER];
char *boundary = " \n"; 
void exec(char** params);
void show();
char no_prompt[3] = "-n";
int flagger = 1;

int main(int argc, char* argv[]){
         
	if(argc == 2 && strcmp(argv[1], no_prompt)==0){
 	flagger = 0;}
	else{
	flagger = 1;}

	while(1) {
	memset(readit,'\0',BUFFER);	
	if(flagger==1){
		show();} 

	 //for thins like control+D
         if(!fgets(readit,BUFFER,stdin)){
          break;}
	 
	 char *finger = readit;	
	 char *ptr = NULL;
	 int i = 0;
	 ptr = strtok(readit,boundary);
	 int oi = strcmp(finger,"|");
	 if(oi != 0){
	 //tokenizing command so that i can use execvp
	 
		while(ptr != NULL){
			
			arguments[i++] = ptr;
			ptr = strtok(NULL,boundary);
				
		}
	 	
	  exec(arguments);}


	//checks to see if there are more metavalues but never worked
	//this one was for testing if there was | but it doesn't work
	else{
	char* line= "|";
	char* tok= NULL;
	int j = 0;	
	tok = strtok (readit,line);
	
	 while(tok != NULL){
	 commands[j++] = tok;
	 tok = strtok(NULL, line);
	}
	
	arguments[0] = strtok(commands[j], " ");
	int count = 0;
	while (arguments[count] != NULL){

		arguments[count++]= strtok(NULL, " ");

	}

	int children = fork();
	if (children == 0){

	 if(children > 0)
	  wait(&children);
	 else if(execvp(arguments[0],arguments)<0)
	  printf("ERROR\n");
	}

	else{ 
	 exit(1);
      	 j++;
	}
	}
}
return 0;

}

void show(){

printf("myshell$ ");
}

// this function is for when only one argument is detected such as ls or cat
void exec(char** params){
		pid_t firstchild=fork();
                        if(firstchild==-1)
                                printf("ERROR\n");
                        else if (firstchild==0){
                               execvp(params[0],params);
               		    }
                        else
                        wait(NULL);
}




