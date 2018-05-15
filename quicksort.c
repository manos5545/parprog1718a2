#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

#define MESSAGES 20

struct message{
	int id;
	char minima[];
};
typedef struct message message;

struct jobQueue{
	message array[MESSAGES];
	int start;
	int finish;
};
typedef struct jobQueue jobQueue;

int QueueEmpty(jobQueue q)
{
	return q.finish==-1;
}

int QueueFull(jobQueue q)
{
	return q.start==(q.finish)%MESSAGES;
}

// global integer buffer
message globalBuffer[MESSAGES];

int global_availmsg = 0;	// empty


pthread_cond_t msg_in = PTHREAD_COND_INITIALIZER;
pthread_cond_t msg_out = PTHREAD_COND_INITIALIZER;

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
    globalBuffer[i].id = i;
    printf("Producer: sending msg %d\n",globalBuffer[i].id);
    global_availmsg = 1;
    
    // signal the receiver that something was put in buffer
    pthread_cond_signal(&msg_in);
    
    // unlock mutex
    pthread_mutex_unlock(&mutex);
  }
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
  pthread_exit(NULL); 
}


int main() {
  
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
