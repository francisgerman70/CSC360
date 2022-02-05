//Francis German
//V00893968
//University of Victoria
//Fall 2021
//CSC 360 Assignment 1

// Used code from stackoverflow, includehelp.com, geeksforgeeks, stackexchange and youtube(code vault)
#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>
#include <string.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <signal.h>   
        


//prototype
void bg(char **, int);
int bglist( int);
void CreateProcess(pid_t,char*);
void bgkill(pid_t, int);
void bgstop(pid_t id);
void bgstart(pid_t id);
void pstat(pid_t id);
void check_zombieProcess();
void remove_process_from_list(pid_t id);



// background process record 
typedef struct Node{
	pid_t id;
	char* word;
	struct Node* next;
	
}Node;
Node* head = NULL;

int main(){
	char* cmdtok;
	char* cmd;
	char *strs[7]= {"bg", "bglist", "bgkill","bgstop","bgstart", "pstat" }; //supported commands
	int Arr_len = sizeof(strs) / sizeof(strs[0]);
	
	

	while(1){	
		int count = 0;
		char *inputArr[20]; // contains user inputs
		int count1 = 0;
		//string token
		cmd = readline("PMan: > ");
		cmdtok = strtok(cmd, " ");
		/* parse the input cmd (e.g., with strtok)
		 */	
		for(int i=0; i<5;i++){
			inputArr[i] = cmdtok;
			cmdtok = strtok(NULL, " ");
		}
		
		for (int i=0;i<Arr_len-1;i++){
			if(strcmp(inputArr[0],strs[i]) == 0){
           count++; 
        }
    }
    if(count == 0){
		printf("PMan: > %s:", cmd);
        printf("\tcommand not found\n");
    }
	   
	if (strcmp(inputArr[0], "bg")==0){
		
			bg(inputArr, 1);
		}
	else if(strcmp(inputArr[0], "bglist")==0){
		
			bglist(count1);
		}	
	else if(strcmp(inputArr[0], "bgkill")==0){
			//convert string to int
			int x = atoi(inputArr[1]);
			bgkill(x,count1);
		}
	else if(strcmp(inputArr[0], "bgstop")==0){
			//convert string to int
			int x = atoi(inputArr[1]);
			bgstop(x);
		}	
	else if(strcmp(inputArr[0], "bgstart")==0){
			//convert string to int
			int x = atoi(inputArr[1]);
			bgstart(x);
		}
	else if(strcmp(inputArr[0], "pstat")==0){
			//convert string to int
			int x = atoi(inputArr[1]);
			pstat(x);
		}	
		check_zombieProcess();
						
	}
		
		
	
	

	return 0;
}
// this command executes a process in background
void bg(char **program, int n) {
	pid_t id;
	id = fork();
	//child process
	if(id == 0){
		if(execvp(program[n], program) < 0){
			perror("Error on execvp");	
		}
		exit(-1);
	}
	
	//parent process
	else if(id > 0) {
		// store information of the background child process in your data structures
		
		CreateProcess(id,program[1]);
		sleep(1);	
	}
	
}

//this function displays a list of programs running in the background
int bglist(int count1) {
	
	count1 = 0;  // Initialize count
    struct Node* curr = head;  // Initialize current
    while (curr != NULL){
        count1++;
		printf("%d:\t %s \n",curr->id,curr->word);
        curr = curr->next;
		
    }
	printf("Total background jobs:%d\n", count1);
	return count1;
}
//this function creates a process in background by storing process in linkedlist
void CreateProcess(pid_t id, char* word){
	
	Node* root = malloc(sizeof(Node));
	root->id = id;
	root->word = word;
	root->next = NULL;
	Node* curr1 = head;
	if (head == NULL) {
		head = root;
	} 
	else {
		while (curr1->next != NULL) {
			curr1 = curr1->next;
		}
		curr1->next = root;
	}
}
//this function sends the SIGKILL signal to a process id and terminates the process in background
void bgkill(pid_t id, int count1){
	// Check if process is in linkedlist 
	int count = 0;
	Node* temp3 = head;
	while (temp3 != NULL) {
		if (temp3->id == id) {
			count++;
		}
		temp3 = temp3->next;
	}
	if (count ==1){
		kill(id, SIGKILL);
		printf("Backgroung job %d has been terminated\n",id);
	}else{
		printf("Process does not exist\n");
	}
	remove_process_from_list(id);
    
    
}
//this function sends the SIGSTOP signal to a process id to stop it temporarily 
void bgstop(pid_t id) {
	// Check if process is in linkedlist 
	int count = 0;
	Node* temp = head;
	while (temp != NULL) {
		if (temp->id == id) {
			count++;
		}
		temp = temp->next;
	}
	if (count ==1){
		kill(id, SIGSTOP);	
		printf("Backgroung job %d has been stopped\n",id);	
	}else{
		printf("Process does not exist\n");
	}
}
//this function sends the SIGCONT signal to a process id to stop it continue execution 
void bgstart(pid_t id) {
	// Check if process is in linkedlist 
	int count = 0;
	Node* temp = head;
	while (temp != NULL) {
		if (temp->id == id) {
			count++;
		}
		temp = temp->next;
	}
	if (count==1){
		kill(id, SIGCONT);
		printf("Backgroung job %d has been resumed\n",id);
		
	}else{
		printf("Process does not exist\n");
	}

}
// this function list information about a process id
void pstat(pid_t id){
	// Check if process is in linkedlist 
	int count = 0;
	Node* temp3 = head;
	while (temp3 != NULL) {
		if (temp3->id == id) {
			count++;
		}
		temp3 = temp3->next;
	}
	if (count ==1){

    	char filename[1000];
    	char filename1[1000];
    	sprintf(filename, "/proc/%d/stat", id);
    	FILE *f = fopen(filename, "r");
		sprintf(filename1, "/proc/%d/status", id);
    	FILE *fp = fopen(filename1, "r");

    	char comm[100];
    	char state;
    	int ppid;
    	float utime;
    	float stime;
    	unsigned int rss;
    	int voluntary_ctxt_switches ;
    	int involuntary_ctxt_switches ;
	

    	fscanf(f, "%*d %s %c %d %*d %*d %*d %*d %*u %*u %*u %*u %*u %f %f %*d %*d %*d %*d %*u %*u %*d %*u %u", comm, &state, &ppid, &utime, &stime, &rss );
		fscanf(f, "%*d %*s %*c %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*D %*d %*D %*d %*D %*d %*d %*d %*d %*d %*d %d %d  ",&voluntary_ctxt_switches, &involuntary_ctxt_switches );
    	utime = utime/ sysconf(_SC_CLK_TCK);
    	stime = stime/ sysconf(_SC_CLK_TCK);
   	 	printf("comm = %s\n", comm);
    	printf("state = %c\n", state);
    	printf("utime = %f\n", utime);
    	printf("stime = %f\n", stime);
    	printf("rss = %u\n", rss);
		  printf("voluntary_ctxt_switches = %d\n", voluntary_ctxt_switches);
    	printf("involuntary_ctxt_switches = %d\n", involuntary_ctxt_switches);
    	fclose(f);
    	fclose(fp);
	}else{
		printf("Invalid process id\n");
	}
}
// this function checks if a process does not executes
void check_zombieProcess(void){
	int status;
	int retVal = 0;
	
	while(1) {
		usleep(1000);
		if(head == NULL){
			return ;
		}
		retVal = waitpid(-1, &status, WNOHANG);
		if(retVal > 0) {
			//remove the background process from your data structure
			if (WIFSIGNALED(status)) {
				printf("Background process %d was killed.\n", retVal);
				remove_process_from_list(retVal);
			}
			if (WIFEXITED(status)) {
				printf("Background process %d terminated.\n", retVal);
				remove_process_from_list(retVal);
			}
		 
		}
		else if(retVal == 0){
			break;
		}
		else{
			perror("waitpid failed");
			exit(EXIT_FAILURE);
		}
	}
	return ;
}
//this function removes a process from data structure
void remove_process_from_list(pid_t id){
	 // Store head node
    Node* temp = head;
	Node* temp2 = NULL;

    while (temp != NULL)
    {
        if (temp->id == id)
        {
            break;
        }

        temp2 = temp;
        temp = temp->next;
    }

    if (temp == NULL)
    {
        printf("");
    }
    else if (temp2 == NULL)
    {
        
        head = temp->next;
    }
    else
    {
        
        temp2->next = temp->next;
    }
   
    free(temp);
  
}
