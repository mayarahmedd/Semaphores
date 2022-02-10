#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
int MaxBufferSize=5;
int *queue;
int front = 0;
int rear = -1;
int CountOfItems =0;
int messagesCount=0;
int item;
sem_t sem_count,empty,full,buf_mutex;

void InsertIntoBuffer (int item)   
{  
    if (CountOfItems != MaxBufferSize)  
    {  
          
    if(rear == MaxBufferSize-1) {
         rear = -1;
      }
           
          rear=rear+1;
          queue[rear]=item;     
          CountOfItems++; 
               
 }  
}
 
int deleteFromBuffer ()  
{   

    item= queue[front]; 
    front=front+1; 
      if(front == MaxBufferSize ) {
      front = 0;
       }
   CountOfItems--;
 
   return item;
}


void* counter(void* arg)
{
  while(true){
    int randomCount = rand() %5+ 1;
    sleep(randomCount);
    int i = *((int *) arg);
    printf("Counter Thread %d: received a message!\n",i);
    printf("Counter Thread %d: waiting to write\n",i);
    sem_wait(&sem_count);
    messagesCount++;
    printf("Counter Thread %d now adding to counter, Counter Value = %d \n",i,messagesCount);
    sem_post(&sem_count);
     }
}

void* monitor(void* arg){
  while(true)
  {
 int randomCount = rand() %7+ 1;
 sleep(randomCount);
 
  int res1;
  printf("Monitor Thread: Waiting to read counter\n"); 
  sem_getvalue(&empty, &res1);
  if(res1==0)
  {
    printf("Monitor Thread: buffer is full...................................... !\n"); 
  }
  sem_wait(&sem_count);
  item = messagesCount;
  printf("Monitor Thread: Reading a count value = %d\n",item );
  messagesCount=0;
  sem_post(&sem_count);
  sem_wait(&empty); 
  sem_wait(&buf_mutex); 
  InsertIntoBuffer(item );
  printf("Monitor Thread: Writing to Buffer at position %d\n",rear);
  sem_post(&buf_mutex);
  sem_post(&full);
  }
      
}

void* collector(void* arg)
{
   while(true){
      int randomCount = rand() %10+ 1;
      sleep(randomCount);
      int res;
      sem_getvalue(&full,&res);
    
      if(res==0){
         printf("collector Thread: buffer is empty.................!\n");
         }
      sem_wait(&full); 
      sem_wait(&buf_mutex); 
      printf("Collector Thread: Reading from Buffer at position %d\n",front); 
      item =deleteFromBuffer();      
      sem_post(&buf_mutex);
      sem_post(&empty);    
     }
}

void init_semaphores() 
{
 sem_init(&sem_count, 0, 1);
 sem_init(&buf_mutex, 0, 1);
 sem_init(&empty,0,MaxBufferSize);
 sem_init(&full,0,0);
}

int main()
{
 int numOfthreads=10;
 //printf("Please Enter Number of Threads: \n");
 //scanf("%d",&numOfthreads);
 
 //printf("Please Enter the Buffer Size\n");
 //scanf("%d",&MaxBufferSize );
 queue= (int*)malloc(MaxBufferSize * sizeof(int)); 
 
 init_semaphores();  
 
 int ptr[numOfthreads];
 pthread_t mCounter[numOfthreads];
 pthread_t mMonitor;
 pthread_t mCollector;
    
 for(int i=0;i<numOfthreads;i++){
      ptr[i] = i;
      pthread_create(&mCounter[i],NULL,counter,&ptr[i]);
    }
    pthread_create(&mMonitor,NULL,monitor,NULL);
    pthread_create(&mCollector,NULL,collector,NULL);

 for(int i=0;i<numOfthreads;i++){
     pthread_join(mCounter[i],NULL);
    }
 pthread_join(mMonitor,NULL);
 pthread_join(mCollector,NULL);

}
