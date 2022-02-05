// Used geekforgeeks, a2-hint as reference and other sites i might have lost track of.
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdbool.h>
#include <time.h>
#include <sys/types.h>

#define TRUE 1
#define FALSE 0

struct Queue* createQueue();
void deQueue(struct Queue* q);
void enQueue(struct Queue* q, int k);
struct QNode* newNode(int k);
double getCurSystemTime();

struct customer_info{ // struct to record the customer information read from customers.txt
  int user_id;
	int class_type;
	int service_time;
	int arrival_time;
};


typedef struct QNode {
	int key;
	struct QNode* next;
    
}QNode;

struct Queue {
	struct QNode *front, *rear;
    int size;
  
}Queue;



struct Queue *class_type_queue[2];
int queue_length[2]; // variable stores the real-time queue length information;
double queue_enter_time;
int queue_status[2]; // variable to record the status of a queue, the value could be idle (not using by any clerk) or the clerk id (1 ~ 4), indicating that the corresponding clerk is now signaling this queue.
bool winner_selected[2] = {false,false}; // variable to record if the first customer in a queue has been successfully selected and left the queue.

// mutexes and condition variable
pthread_mutex_t lock[2];
pthread_mutex_t queue_mutex[2];
pthread_mutex_t clerk_mutex[5];
pthread_cond_t clerk_variable[5];
pthread_cond_t queue_variable[2];
pthread_mutexattr_t recursiveMutexAttributes;






double start_time;
double end_time;
double overall_waiting_time; //A global variable to add up the overall waiting time for all customers, every customer add their own waiting time to this variable, mutex_lock is necessary.
double business_ave;
double economy_ave;
int customerNum;
double time1;
struct timeval init_time;
int queue_id;
int user_id;
int class_type;
int arrival_time;
int service_time;
double cusBusNum;
double cusEcoNum;
int clerk_woke_me_up;




/* customer thread function */
void *customer_entry(void *cus_info){
  struct customer_info * p_myInfo = (struct customer_info *) cus_info;
	usleep(p_myInfo->arrival_time * 100000);
	
    fprintf(stdout, "A customer arrives: customer ID %2d. \n", p_myInfo->user_id);
  
  /* mutexLock of selected queue */
  if(pthread_mutex_lock(&queue_mutex[p_myInfo->class_type])!=0){
    printf("Error locking mutex.\n");
		exit(0);
	
  }
  //critical section
	/* Enqueue operation: get into either business queue or economy queue by using p_myInfo->class_type*/
    class_type_queue[p_myInfo->class_type]= createQueue();
    enQueue(class_type_queue[p_myInfo->class_type], p_myInfo->user_id);
    queue_enter_time = getCurSystemTime();
    queue_length[p_myInfo->class_type]++;
    queue_id = p_myInfo->class_type;
    
    
    fprintf(stdout, "A customer enters a queue: the queue ID %1d, and length of the queue %2d. \n", p_myInfo->class_type, queue_length[p_myInfo->class_type]);
    
    
    
    while (TRUE) {
            //pthread_cond_wait(&queue_variable[p_myInfo->class_type], &queue_mutex[p_myInfo->class_type]);
			if (p_myInfo->user_id == class_type_queue[p_myInfo->class_type]->front->key && (winner_selected[0] == FALSE || winner_selected[1] == FALSE)) {
            deQueue(class_type_queue[p_myInfo->class_type]);
				queue_length[p_myInfo->class_type]--;
				winner_selected[p_myInfo->user_id] = TRUE; // update the winner_selected variable to indicate that the first customer has been selected from the queue
				break; 
                
			}
               
                
		}
  
  
  if(pthread_mutex_unlock(&queue_mutex[p_myInfo->class_type]) != 0){
    printf("Error unlocking mutex.\n");
		exit(0);
  }
  
	
	/* Try to figure out which clerk awoken me, because you need to print the clerk Id information */
	usleep(10); // Add a usleep here to make sure that all the other waiting threads have already got back to call pthread_cond_wait. 10 us will not harm your simulation time.
	 clerk_woke_me_up = queue_status[p_myInfo->class_type];
    
	
  
  
  

    // get time for both economy and business class
    /* get the current machine time; updates the overall_waiting_time*/
    if (queue_status[p_myInfo->class_type] == 0) {
        if(pthread_mutex_lock(&lock[0])){
          printf("Error locking mutex.\n");
		        exit(0);
        }
        // critical section
        economy_ave = (economy_ave + getCurSystemTime()) - p_myInfo->service_time* 0.01;
        if(pthread_mutex_unlock(&lock[0])){
          printf("Error unlocking mutex.\n");
		        exit(0);
        }
    }else {
        if(pthread_mutex_lock(&lock[1])){
          printf("Error locking mutex.\n");
		        exit(0);
        }
        business_ave = (business_ave + getCurSystemTime()) - p_myInfo->service_time* 0.01;
        if(pthread_mutex_unlock(&lock[1])){
          printf("Error unlocking mutex.\n");
		      exit(0);
        }
    }
	
	fprintf(stdout, "A clerk starts serving a customer: start time %.2f, the customer ID %2d, the clerk ID %1d. \n",queue_enter_time, p_myInfo->user_id, clerk_woke_me_up);
	
	usleep(p_myInfo->service_time* 100000);

  end_time = getCurSystemTime();
  
	
	
	fprintf(stdout, "A clerk finishes serving a customer: end time %.2f, the customer ID %2d, the clerk ID %1d. \n", end_time, p_myInfo->user_id, clerk_woke_me_up);
 //Lock mutex
	if (pthread_mutex_lock(&clerk_mutex[clerk_woke_me_up])!= 0) {
        printf("Error: pthread_mutex_lock failed");
        exit(-1);
    }
    
    //Critical Section
    if (pthread_cond_broadcast(&clerk_variable[clerk_woke_me_up]) != 0) {
        printf("Error pthread_cond_signal failed");
        exit(-1);
    }
    //unlock mutex
    if (pthread_mutex_unlock(&clerk_mutex[clerk_woke_me_up]) != 0) {
        printf("Error pthread_mutex_unlock failed ");
        exit(-1);
    }
	
	
	pthread_exit(NULL);
	
	return NULL;
}

  
// function entry for clerk threads
void *clerk_entry(void * clerkNum){
  int clerkID;
  
	
  clerkID = *((int *)clerkNum);
  
	while(TRUE){
        
		
		/* selected_queue_ID = Select the queue based on the priority and current customers number */
    if(queue_id == 0){
    if(pthread_mutex_lock(&queue_mutex[0])){
    printf("Error locking mutex.195\n");
		exit(0);
    }
    
    queue_status[0] = clerkID;
    
    pthread_cond_broadcast(&queue_variable[0]);
    winner_selected[0] = FALSE;

    if(pthread_mutex_unlock(&queue_mutex[0])){
    printf("Error unlocking mutex.203\n");
		exit(0);
  }
    }else{
        if(pthread_mutex_lock(&queue_mutex[1])){
    printf("Error locking mutex.195\n");
		exit(0);
    }
    
    queue_status[1] = clerkID;
    
    pthread_cond_broadcast(&queue_variable[1]);
    winner_selected[1] = FALSE;

    if(pthread_mutex_unlock(&queue_mutex[1])){
    printf("Error unlocking mutex.203\n");
		exit(0);
  }

    }
    
  
    
    // lock clerk mutex
    if(clerkID == 1){
        if (pthread_mutex_lock(&clerk_mutex[0]) != 0) {
           printf("Error: pthread_mutex_ailed.29\n");
            exit(-1);
        }
      
        /* critical section */
        
        if (pthread_cond_wait(&clerk_variable[0], &clerk_mutex[0]) != 0) {
            printf("Error: pthread_cond_waitfailed 25.\n");
            exit(-1);
        }
        

        // unlock clerk mutex
        if (pthread_mutex_unlock(&clerk_mutex[0]) != 0) {
           printf("error: pthread_mutex_unlock clerk_mutex failed.221\n");
            exit(-1);
          }
    }
    else if(clerkID == 2){
        if (pthread_mutex_lock(&clerk_mutex[1]) != 0) {
           printf("Error pthread_mutex_lock_clerk failed.209\n");
            exit(-1);
        }
        
      
        /* critical section to wait  */
        
        if (pthread_cond_wait(&clerk_variable[1], &clerk_mutex[1]) != 0 ) {
            printf("Error pthread_cond_wait clerk failed 215.\n");
            exit(-1);
        }
        

        // unlock clerk mutex
        if (pthread_mutex_unlock(&clerk_mutex[1]) != 0) {
           printf("Error pthread_mutex_failed.221\n");
            exit(-1);
          }
    }
    else if(clerkID == 3){
        if (pthread_mutex_lock(&clerk_mutex[2])!= 0) {
           printf("Error pthread_mutex_failed.20\n");
            exit(-1);
        }
        
      
        /* critical section */
        
        if (pthread_cond_wait(&clerk_variable[2], &clerk_mutex[2]) != 0) {
            printf("Error pthread_cond_waitfailed 15.\n");
            exit(-1);
        }
        

        // unlock clerk mutex
        if (pthread_mutex_unlock(&clerk_mutex[2]) != 0) {
           printf("Error: pthread_mutex_failed.21\n");
            exit(-1);
          }
    }
    else {
        if (pthread_mutex_lock(&clerk_mutex[3])!= 0) {
           printf("Error pthread_mutex_failed9\n");
            exit(-1);
        }
        
      
      
        /* critical section  */
        
        if (pthread_cond_wait(&clerk_variable[3], &clerk_mutex[3]) != 0) {
            printf("Error pthread_cond_wait 1.\n");
            exit(-1);
        }
        

        // unlock clerk mutex
        if (pthread_mutex_unlock(&clerk_mutex[3])!= 0) {
           printf("Error pthread_mutex_failed.22\n");
            exit(-1);
          }
    }

        
    
    
    
//Could not get clerk thread to work properly (kept getting deadlocks).
    }
	pthread_exit(NULL);
	
	return NULL;
}




int main(int argc, char *argv[]){
  FILE *fp;
  char *filename;
  char ch[99];

  
    //initializing mutex,con_var and mutexattr
    if(pthread_mutexattr_init(&recursiveMutexAttributes)!=0){
    printf("Error initialzing mutexattr.\n");
		exit(0);
  }
    pthread_mutexattr_settype(&recursiveMutexAttributes, PTHREAD_MUTEX_RECURSIVE);

  if(pthread_mutex_init(&queue_mutex[0], &recursiveMutexAttributes)!=0){
  printf("Error initialzing mutex.\n");
		exit(0);
  }
  if(pthread_mutex_init(&queue_mutex[1], &recursiveMutexAttributes)!=0){
  printf("Error initialzing mutex.\n");
		exit(0);
  }
  if(pthread_mutex_init(&clerk_mutex[0], &recursiveMutexAttributes)!=0){
  printf("Error initialzing mutex.\n");
		exit(0);
  }
  if(pthread_mutex_init(&clerk_mutex[1], &recursiveMutexAttributes)!=0){
  printf("Error initialzing mutex.\n");
		exit(0);
  }
  if(pthread_mutex_init(&clerk_mutex[2], &recursiveMutexAttributes)!=0){
  printf("Error initialzing mutex.\n");
		exit(0);
  }
  if(pthread_mutex_init(&clerk_mutex[3], &recursiveMutexAttributes)!=0){
  printf("Error initialzing mutex.\n");
		exit(0);
  }
  if(pthread_mutex_init(&clerk_mutex[4], &recursiveMutexAttributes)!=0){
  printf("Error initialzing mutex.\n");
		exit(0);
  }
  if(pthread_cond_init(&queue_variable[0], NULL)!=0){
  printf("Error initialzing cond_var.\n");
		exit(0);
  }
  if(pthread_cond_init(&queue_variable[1], NULL)!=0){
  printf("Error initialzing cond_var.\n");
		exit(0);
  }
  if(pthread_cond_init(&clerk_variable[0], NULL)!=0){
  printf("Error initialzing cond_var.\n");
		exit(0);
  }
  if(pthread_cond_init(&clerk_variable[1], NULL)!=0){
  printf("Error initialzing cond_var.\n");
		exit(0);
  }
  if(pthread_cond_init(&clerk_variable[2], NULL)!=0){
  printf("Error initialzing cond_var.\n");
		exit(0);
  }
  if(pthread_cond_init(&clerk_variable[3], NULL)!=0){
  printf("Error initialzing cond_var.\n");
		exit(0);
  }
  if(pthread_cond_init(&clerk_variable[4], NULL)!=0){
  printf("Error initialzing cond_var.\n");
		exit(0);
  }
  if(pthread_mutex_init(&lock[0], NULL)!=0){
  printf("Error initialzing mutex.\n");
		exit(0);
  }
  if(pthread_mutex_init(&lock[1], NULL)!=0){
  printf("Error initialzing mutex.\n");
		exit(0);
  }
  
  
  
  

  // Check if a filename has been specified in the command
  if (argc < 2)
   {
        printf("Missing Filename\n");
        return(1);
   }
   else
  {
        filename = argv[1];
        printf("Filename : %s\n", filename);
   }

   // Open file in read-only mode
   fp = fopen(filename,"r");

       if (NULL == fgets(ch, sizeof(ch), fp)) {
        printf("Error\n");
        exit(-1);
    }
        
    customerNum = atoi(ch); //number of customers
    struct customer_info custom_info[customerNum]; //customer array
    pthread_t customId[customerNum]; //thread identifier array
    int clerk_info[5]; //clerk array
    pthread_t clerkId[5]; //thread identifier array
    
    
	  
    for (int i=0; i<customerNum; i++) {
        
        fgets(ch, sizeof(ch), fp);
        (user_id = atoi(strtok(ch, ":")));
        
        (class_type = atoi(strtok(NULL, ",")));
        (arrival_time = atoi(strtok(NULL, ",")));
        (service_time = atoi(strtok(NULL, ",")));

        custom_info[i].user_id = user_id;
        custom_info[i].class_type = class_type;
        custom_info[i].arrival_time = arrival_time;
        custom_info[i].service_time = service_time;
    }

    
   
// get number of business and economy customer
  for (int i =0 ; i<customerNum; i++){
    if(custom_info[i].class_type == 0){
      cusEcoNum++;
    }else{
      cusBusNum++;
    }
  }
    
    gettimeofday(&init_time, NULL);
    time1 = (init_time.tv_sec + (double) init_time.tv_usec / 1000000);

   //create clerk thread
    for (int i=1; i<5; i++) {
        clerk_info[i] = i;
        
      
      pthread_create(&clerkId[i], NULL, clerk_entry, (void *)&clerk_info[i]);
                
    }
    

    //create customer thread  
  for(int i = 0; i < customerNum; i++){ // number of customers
		pthread_create(&customId[i], NULL, customer_entry, (void *)&custom_info[i]); //custom_info: passing the customer information (e.g., customer ID, arrival time, service time, etc.) to customer thread
	}
  
	// wait for all customer threads to terminate
	for (int i = 0; i < customerNum; i++) {
		if (pthread_join(customId[i], NULL) != 0) {
			printf("Error: failed to join pthread.\n");
			exit(1);
		}
	}
  
  // calculate the average waiting time of all customers
  overall_waiting_time = (economy_ave + business_ave)/customerNum;
  double buisness = business_ave /cusBusNum;
  double economy = economy_ave / cusEcoNum;
  
    printf("The average waiting time for all customers in the system is: %.2f seconds. \n", overall_waiting_time);
    printf("The average waiting time for all business-class customers is: %.2f seconds. \n", buisness);
    printf("The average waiting time for all economy-class customers is: %.2f seconds. \n", economy);
}
// A utility function to create a new linked list node.
struct QNode* newNode(int k)
{
	struct QNode* temp = (struct QNode*)malloc(sizeof(QNode));
	temp->key = k;
	temp->next = NULL;
	return temp;
}
// A utility function to create an empty queue
struct Queue* createQueue(){
	struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
	q->front = q->rear = NULL;
  q->size = 0;
	return q;
}

// The function to add a key k to q
void enQueue(struct Queue* q, int k){
	// Create a new LL node
	struct QNode* temp = newNode(k);

	// If queue is empty, then new node is front and rear both
	if (q->rear == NULL) {
		q->front = q->rear = temp;
		return;
	}

	// Add the new node at the end of queue and change rear
	q->rear->next = temp;
	q->rear = temp;
    q->size = q->size + 1;
}

// Function to remove a key from given queue q
void deQueue(struct Queue* q)
{
	// If queue is empty, return NULL.
	if (q->front == NULL)
		return;

	// Store previous front and move front one node ahead
	struct QNode* temp = q->front;

	q->front = q->front->next;

	// If front becomes NULL, then change rear also as NULL
	if (q->front == NULL)
		q->rear = NULL;
    q->size = q->size - 1;


	free(temp);
}

double getCurSystemTime(){
  struct timeval curr;
  gettimeofday(&curr, NULL);
  double cur = (curr.tv_sec + (double)curr.tv_usec / 1000000);

  return cur - time1;
}