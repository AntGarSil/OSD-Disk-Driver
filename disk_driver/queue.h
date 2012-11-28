/* This Queue implementation of singly linked list in C implements 3 
 * operations: add, remove and print elements in the list.  Well, actually, 
 * it implements 4 operations, lats one is list_free() but free() should not 
 * be considered the operation but  a mandatory practice like brushing 
 * teeth every morning, otherwise you will end up loosing some part of 
 * your body(the software) Its is the modified version of my singly linked 
 * list suggested by Ben from comp.lang.c . I was using one struct to do 
 * all the operations but Ben added a 2nd struct to make things easier and 
 * efficient.
 *
 * I was always using the strategy of searching through the list to find the
 *  end and then addd the value there. That way list_add() was O(n). Now I 
 * am keeping track of tail and always use  tail to add to the linked list, so 
 * the addition is always O(1), only at the cost of one assignment.
 *
 *
 * VERISON 0.5
 *
 */
#ifndef _QUEUE_H_
#define _QUEUE_H_

#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>

struct my_struct
{
  void *data;
  struct my_struct* next;
};


struct queue
{
  struct my_struct* head;
  struct my_struct* tail;
};

/* enqueues an element */
struct queue* enqueue( struct queue*, void * data);
/* dequeues an element */
void* dequeue( struct queue*);
/* returns 1 if the queue is empty and 0 otherwise*/
int queue_empty ( struct queue* s );
/* If it finds the data in the queue it removes it and returns it. Otherwise it returns NULL */
void* queue_find_remove(struct queue* s, void * data );

struct queue* queue_new(void);
struct queue* queue_free( struct queue* );

void queue_print(struct queue* );
void queue_print_element(struct my_struct* );

#endif


