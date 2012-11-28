#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "dev.h"
#include "queue.h"


/* Demonstrates and tests enqueuing requests to a queue and removing them. */ 

void test_queue_find_remove(struct queue *q, struct aio_rq* e) {
  struct aio_rq *t;
  printf("******\nRemoving request: Thread ID = %d  Type = %s Offset = %d \n", e->tid, (e->type==READ_RQ)?"Read":"Write", e->offset); 
  t = queue_find_remove(q,e);
  queue_print(q);
  if (t)
    printf("Removed request: Thread ID = %d  Type = %s Offset = %d \n******\n\n", t->tid, (t->type==READ_RQ)?"Read":"Write", t->offset);
  
}

int main(int argc, char *argv[])
{
  struct aio_rq *p, *r, *s;
  struct queue* q = queue_new();

  p = (struct aio_rq *)malloc (sizeof (struct aio_rq));
  r = (struct aio_rq *)malloc (sizeof (struct aio_rq));
  s = (struct aio_rq *)malloc (sizeof (struct aio_rq));


  p->type = WRITE_RQ;
  p->offset = 0;
  p->tid = 0;
  
  r->type = WRITE_RQ;
  r->offset = 4;
  r->tid = 1;
  
  s->type = READ_RQ;
  s->offset = 8;
  s->tid = 2;

  enqueue(q,p);
  queue_print(q);
  enqueue(q,r);
  queue_print(q);
  enqueue(q,s);
  queue_print(q);

  test_queue_find_remove(q, r);
  test_queue_find_remove(q, s);
  test_queue_find_remove(q, p);
  
  enqueue(q,s);  
  queue_print(q);
  test_queue_find_remove(q, s);


  return 0;
} /****** End main() ******/


