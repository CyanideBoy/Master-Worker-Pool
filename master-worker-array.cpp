#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <iostream>
#include <climits>
#include <iostream>

using namespace std;

int item_to_produce,item_consumed;                  // INITIALIZE THESE NIGGAS
int total_items, max_buf_size, num_workers;
int* buff;

sem_t empty;
sem_t full;
sem_t lock; // LOCK FOR array access

bool flag = false;

void print_produced(int num) {

  printf("Produced %d\n", num);
}


void print_consumed(int num, int worker) {

  printf("Consumed %d by worker %d\n", num, worker);
  
}


void *generate_requests_loop(void *data)
{
  int thread_id = *((int *)data);

  while(1)
    {
      if(item_to_produce >= total_items)
	    break;

      sem_wait(&empty);
      print_produced(item_to_produce);
      buff[item_to_produce%max_buf_size] = item_to_produce;
      item_to_produce++;
      sem_post(&full);
    }


    sem_wait(&empty);
    buff[item_to_produce%max_buf_size] = INT_MAX;
    sem_post(&full);

  return 0;
}


void *consume_loop(void *data)
{
  int thread_id = *((int *)data);

  while(1)
    {
        sem_wait(&lock);
      if(item_consumed >= total_items)
      {
          for(int i=0;i<num_workers;i++)
          {
              sem_post(&full);
          }
          break;
      }
	    

        sem_post(&lock);


        sem_wait(&full);
      
        sem_wait(&lock);
      
      if(buff[item_consumed%max_buf_size]!= INT_MAX)
      {
        print_consumed(buff[item_consumed%max_buf_size],thread_id);
        item_consumed++;
        sem_post(&lock);
        sem_post(&empty);
      }

      else
      {
          sem_post(&lock);
          for(int i=0;i<num_workers;i++)
          {
              sem_post(&full);
          }          
          //cout<<"1"<<endl;
          return 0;
      }
      

    }
  sem_post(&lock);
  return 0;
}


int main(int argc, char *argv[])
{
 
    int master_thread_id = 0;
    pthread_t master_thread;

    if (argc < 4) 
    {
        printf("./master-worker #total_items #max_buf_size #num_workers e.g. ./exe 10000 1000 4\n");
        exit(1);
    }
    else
    {
        num_workers = atoi(argv[3]);
        total_items = atoi(argv[1]);
        max_buf_size = atoi(argv[2]);
    }
    
    buff = new int[max_buf_size];
    
    item_to_produce = 0;
    item_consumed = 0;


    int* w_id = new int[num_workers];
    pthread_t* w_thread = new pthread_t[num_workers];

    for(int i=0;i<num_workers;i++)
    {
        w_id[i] = i;
    }

    sem_init(&empty,0,max_buf_size);
    sem_init(&full,0,0);
    sem_init(&lock,0,1);


    pthread_create(&master_thread, NULL, generate_requests_loop, (void *)&master_thread_id);

    for(int i=0;i<num_workers;i++)
    {
        pthread_create(&w_thread[i], NULL, consume_loop, (void *)&(w_id[i]));
    }

    pthread_join(master_thread, NULL);
    printf("master joined\n");


    for(int i=0;i<num_workers;i++)
    {
        pthread_join(w_thread[i], NULL);
        //printf("worker number %d joined\n",w_id[i]);
    }
    
    delete[] w_id;
    delete[] w_thread;
    delete[] buff;
    

    return 0;
}


/*
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

int item_to_produce,item_consumed;                  // INITIALIZE THESE NIGGAS
int total_items, max_buf_size, num_workers;
int* buff;
bool flag = false;
//int prod,cons;

sem_t empty;
sem_t full;
sem_t lock; // LOCK FOR array access
sem_t LFLAG;



void print_produced(int num) {

  printf("Produced %d\n", num);
}


void print_consumed(int num, int worker) {

  printf("Consumed %d by worker %d\n", num, worker);
  
}


void *generate_requests_loop(void *data)
{
  int thread_id = *((int *)data);

  while(1)
    {
      if(item_to_produce >= total_items)
	    break;

      sem_wait(&empty);
      sem_wait(&lock);
      print_produced(item_to_produce);
      buff[item_to_produce%max_buf_size] = item_to_produce;
      item_to_produce++;
      sem_post(&lock);
      sem_post(&full);
    }
  return 0;
}


void *consume_loop(void *data)
{
  int thread_id = *((int *)data);

  while(1)
    {
      if(item_consumed >= total_items)
	    break;
      
      sem_wait(&full);

      sem_wait(&LFLAG);
      if(flag = true){return 0;}
      sem_post(&LFLAG);

      sem_wait(&lock);
      print_consumed(buff[item_consumed%max_buf_size],thread_id);
      item_consumed++;
      sem_post(&lock);
      sem_post(&empty);

    }
  return 0;
}


int main(int argc, char *argv[])
{
 
    int master_thread_id = 0;
    pthread_t master_thread;
    item_to_produce = 0;

    if (argc < 4) 
    {
        printf("./master-worker #total_items #max_buf_size #num_workers e.g. ./exe 10000 1000 4\n");
        exit(1);
    }
    else
    {
        num_workers = atoi(argv[3]);
        total_items = atoi(argv[1]);
        max_buf_size = atoi(argv[2]);
    }
    
    buff = new int[max_buf_size];
    
    item_to_produce = 0;
    item_consumed = 0;
    //prod = 0;
    //cons = 0;

    int* w_id = new int[num_workers];
    pthread_t* w_thread = new pthread_t[num_workers];

    for(int i=0;i<num_workers;i++)
    {
        w_id[i] = i;
    }

    sem_init(&empty,0,max_buf_size);
    sem_init(&full,0,0);
    sem_init(&lock,0,1);
    sem_init(&LFLAG,0,1);


    pthread_create(&master_thread, NULL, generate_requests_loop, (void *)&master_thread_id);

    for(int i=0;i<num_workers;i++)
    {
        pthread_create(&w_thread[i], NULL, consume_loop, (void *)&w_id[i]);
    }

    pthread_join(master_thread, NULL);
    printf("master joined\n");

    sem_wait(&LFLAG);
    flag = true;
    sem_post(&LFLAG);

    for(int i=0;i<num_workers;i++)
    {
        pthread_join(w_thread[i], NULL);
        printf("worker number %d joined\n",w_id[i]);
    }
    //deallocate and free up any memory you allocated

    return 0;
}
*/




/*
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

int item_to_produce,item_consumed;                  // INITIALIZE THESE NIGGAS
int total_items, max_buf_size, num_workers;
int* buff;
//int prod,cons;

sem_t empty;
sem_t full;
sem_t lock; // LOCK FOR array access



void print_produced(int num) {

  printf("Produced %d\n", num);
}


void print_consumed(int num, int worker) {

  printf("Consumed %d by worker %d\n", num, worker);
  
}


void *generate_requests_loop(void *data)
{
  int thread_id = *((int *)data);

  while(1)
    {
      if(item_to_produce >= total_items)
	    break;

      sem_wait(&empty);
      sem_wait(&lock);
      print_produced(item_to_produce);
      buff[item_to_produce%max_buf_size] = item_to_produce;
      item_to_produce++;
      sem_post(&lock);
      sem_post(&full);
    }
  return 0;
}


void *consume_loop(void *data)
{
  int thread_id = *((int *)data);

  while(1)
    {
      if(item_consumed >= total_items)
	    break;
      
      sem_wait(&full);
      sem_wait(&lock);
      print_consumed(buff[item_consumed%max_buf_size],thread_id);
      item_consumed++;
      sem_post(&lock);
      sem_post(&empty);
      usleep(1009);
    }
  return 0;
}


int main(int argc, char *argv[])
{
 
    int master_thread_id = 0;
    pthread_t master_thread;
    item_to_produce = 0;

    if (argc < 4) 
    {
        printf("./master-worker #total_items #max_buf_size #num_workers e.g. ./exe 10000 1000 4\n");
        exit(1);
    }
    else
    {
        num_workers = atoi(argv[3]);
        total_items = atoi(argv[1]);
        max_buf_size = atoi(argv[2]);
    }
    
    buff = new int[max_buf_size];
    
    item_to_produce = 0;
    item_consumed = 0;
    //prod = 0;
    //cons = 0;

    int* w_id = new int[num_workers];
    pthread_t* w_thread = new pthread_t[num_workers];

    for(int i=0;i<num_workers;i++)
    {
        w_id[i] = i;
    }

    sem_init(&empty,0,max_buf_size);
    sem_init(&full,0,0);
    sem_init(&lock,0,1);


    pthread_create(&master_thread, NULL, generate_requests_loop, (void *)&master_thread_id);

    for(int i=0;i<num_workers;i++)
    {
        pthread_create(&w_thread[i], NULL, consume_loop, (void *)&w_id[i]);
    }

    pthread_join(master_thread, NULL);
    printf("master joined\n");

    for(int i=0;i<num_workers;i++)
    {
        pthread_join(w_thread[i], NULL);
    }
    //deallocate and free up any memory you allocated

    return 0;
}
*/