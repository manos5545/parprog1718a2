#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

#define MESSAGES 20
#define numOfThreads 8
#define N 10000
#define CUTOFF 1000



struct message{
	int startOfArray;
	int finishOfArray;
};
typedef struct message message;

struct jobQueue{
	message array[MESSAGES];
	int start;
	int finish;
};
typedef struct jobQueue jobQueue;

void QueueInit(jobQueue *q)
{
	q->start = 0;
	q->finish = 0;
}

int QueueEmpty(jobQueue q)
{
	return q.finish==-1;
}

int QueueFull(jobQueue q)
{
	return q.start==(q.finish)%MESSAGES;
}

int QueueEnqueue(jobQueue *q, message x)
{
	if(QueueFull(*q))
		return 1;
	else
	{
			
		if(QueueEmpty(*q))
		{
			q->start=0;
			q->finish=0;
		}
		else
		{
			q->finish=(q->finish+1);
		}
		q->array[q->finish]=x;
		return 0;
	}
}

void inssort(double *a,int n) {
int i,j;
double t;
  
  for (i=1;i<n;i++) {
    j = i;
    while ((j>0) && (a[j-1]>a[j])) {
      t = a[j-1];  a[j-1] = a[j];  a[j] = t;
      j--;
    }
  }

}


void quicksort(double *a,int n) {
	
int first,last,middle;
double t,p;
int i,j;

  // check if below cutoff limit
  if (n<=CUTOFF) {
    inssort(a,n);
    message oloklirosi;
    oloklirosi.startOfArray = 0;
    oloklirosi.finishOfArray = n-1;
    printf("Sort completed on partition %d and %d", oloklirosi.startOfArray, oloklirosi.finishOfArray);
    return;
  }
  
  // take first, last and middle positions
  first = 0;
  middle = n/2;
  last = n-1;  
  
  // put median-of-3 in the middle
  if (a[middle]<a[first]) { t = a[middle]; a[middle] = a[first]; a[first] = t; }
  if (a[last]<a[middle]) { t = a[last]; a[last] = a[middle]; a[middle] = t; }
  if (a[middle]<a[first]) { t = a[middle]; a[middle] = a[first]; a[first] = t; }
    
  // partition (first and last are already in correct half)
  p = a[middle]; // pivot
  for (i=1,j=n-2;;i++,j--) {
    while (a[i]<p) i++;
    while (p<a[j]) j--;
    if (i>=j) break;

    t = a[i]; a[i] = a[j]; a[j] = t;      
  }
   
  // recursively sort halves
  quicksort(a,i);
  quicksort(a+i,n-i);
  
}

void *ThreadJob(void *args){
	quicksort(args, N);
}
// global integer buffer
message globalBuffer[MESSAGES];
// global avail messages count (0 or 1)
int global_availmsg = 0;	// empty

// condition variable, signals a put operation (receiver waits on this)
pthread_cond_t msg_in = PTHREAD_COND_INITIALIZER;
// condition variable, signals a get operation (sender waits on this)
pthread_cond_t msg_out = PTHREAD_COND_INITIALIZER;

// mutex protecting common resources
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


// producer thread function
void *producer_thread(void *args) {
  int i;
  message minima;
  
  // send a predefined number of messages
  for (i=0;i<MESSAGES;i++) {
    // lock mutex
    pthread_mutex_lock(&mutex);
    while (global_availmsg>0) {	// NOTE: we use while instead of if! more than one thread may wake up
    				
      pthread_cond_wait(&msg_out,&mutex);  // wait until a msg is received - NOTE: mutex MUST be locked here.
      					   // If thread is going to wait, mutex is unlocked automatically.
      					   // When we wake up, mutex will be locked by us again. 
    }
    // send message
    globalBuffer[i].startOfArray = i;
    printf("Producer: sending msg %d\n",globalBuffer[i].startOfArray);
    global_availmsg = 1;
    
    // signal the receiver that something was put in buffer
    pthread_cond_signal(&msg_in);
    
    // unlock mutex
    pthread_mutex_unlock(&mutex);
  }
  
  // exit and let be joined
  pthread_exit(NULL); 
}
  
  
// receiver thread function
void *consumer_thread(void *args) {
  int i;
  
  // receive a predefined number of messages
  for (i=0;i<MESSAGES;i++) {
    // lock mutex
    pthread_mutex_lock(&mutex);
    while (global_availmsg<1) {	// NOTE: we use while instead of if! more than one thread may wake up
      pthread_cond_wait(&msg_in,&mutex); 
    }
    // receive message
    printf("Consumer: received msg %d\n",globalBuffer[i]);
    global_availmsg = 0;
    
    // signal the sender that something was removed from buffer
    pthread_cond_signal(&msg_out);
    
    // unlock mutex
    pthread_mutex_unlock(&mutex);
  }
  
  // exit and let be joined
  pthread_exit(NULL); 
}

int main() {
  double *a;
  int i;
  jobQueue *myQueue;
  //mallocing array
  a = (double *)malloc(N*sizeof(double));
  if (a==NULL) {
    printf("error in malloc\n");
    exit(1);
  }
  message firstMessage;
  firstMessage.startOfArray = 0;
  firstMessage.finishOfArray = N;
  QueueEnqueue(myQueue, firstMessage);

  // fill array with random numbers
  srand(time(NULL));
  for (i=0;i<N;i++) {
    a[i] = (double)rand()/RAND_MAX;
  }
  
  //Creating the threadpool
  pthread_t threads[numOfThreads];
  int t;
  for (t=0; t<numOfThreads; t++){
    pthread_create(&(threads[t]), NULL, NULL, NULL);
  }
  
  pthread_t producer,consumer;
  
  // create threads
  pthread_create(&producer,NULL,producer_thread,NULL);
  pthread_create(&consumer,NULL,consumer_thread,NULL);
  
  // then join threads
  pthread_join(producer,NULL);
  pthread_join(consumer,NULL);

  // destroy mutex - should be unlocked
  pthread_mutex_destroy(&mutex);

  // destroy cvs - no process should be waiting on these
  pthread_cond_destroy(&msg_out);
  pthread_cond_destroy(&msg_in);

  return 0;
}
