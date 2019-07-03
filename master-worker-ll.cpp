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

struct buf{
    int val;
    buf* nxt;
    buf* prev;

    buf(){
        val=-1;
        nxt = NULL;
        prev = NULL;
    }

};

class ds{
    public:
    buf* head;
    buf* tail;
    int num;
    ds(){
        head = NULL;
        num =0;
    }

    void add(int data)
    {
        buf* ptr = new buf;
        ptr->prev = head;
        ptr->nxt = NULL;
        ptr->val = data;
        
        if(num>0)
        {
            head->nxt = ptr;
        }
        else
        {
            tail = ptr;
        }
        
        head = ptr;
        num++;

        printf("Produced %d\n", data);
    }

    void poison_add(int data)
    {
        buf* ptr = new buf;
        ptr->prev = head;
        ptr->nxt = NULL;
        ptr->val = data;
        
        if(num>0)
        {
            head->nxt = ptr;
        }
        else
        {
            tail = ptr;
        }
        
        head = ptr;
        num++;

    }


    int get_data()
    {
       return tail->val;
    }

    void consume(int worker)
    {
        int data = tail->val;
        
        if(num == 1)
        {
            tail = NULL;
            num = 0;
            delete head;
            head = NULL;
        }
        else
        {
            //tail->(nxt->prev) = NULL;
            //buf* x = tail;
            //x ->
            //delete tail;

            tail = tail->nxt;
            delete tail->prev;
            tail->prev = NULL;
            num--;
        }
        printf("Consumed %d by worker %d\n", data, worker);
    }


};

ds list;
int item_to_produce,item_consumed;                  // INITIALIZE THESE NIGGAS
int total_items, max_buf_size, num_workers;

sem_t empty;
sem_t full;
sem_t lock; // LOCK FOR array access

void *generate_requests_loop(void *data)
{
  int thread_id = *((int *)data);

  while(1)
    {
      if(item_to_produce >= total_items)
	    break;

      
      sem_wait(&empty);
      sem_wait(&lock);
      list.add(item_to_produce);
      item_to_produce++;
      sem_post(&full);
      sem_post(&lock);
    }


    sem_wait(&empty);
    sem_wait(&lock);
    list.poison_add(INT_MAX);
    sem_post(&full);
    sem_post(&lock);

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
      
      if((list.get_data())!= INT_MAX)
      {
        list.consume(thread_id);
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
    

    return 0;
}

