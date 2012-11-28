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

#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>

#include "queue.h"
#include "dev.h"

/* Will always return the pointer to queue */
struct queue* enqueue(struct queue* s, void * i)
{
  struct my_struct* p = malloc( 1 * sizeof(*p) );

  if( NULL == p )
    {
      fprintf(stderr, "IN %s, %s: malloc() failed\n", __FILE__, "list_add");
      return s; 
    }

  p->data = i;
  p->next = NULL;


  if( NULL == s )
    {
      printf("Queue not initialized\n");
      free(p);
      return s;
    }
  else if( NULL == s->head && NULL == s->tail )
    {
      /* printf("Empty list, adding p->data: %d\n\n", p->data);  */
      s->head = s->tail = p;
      return s;
    }
  else if( NULL == s->head || NULL == s->tail )
    {
      fprintf(stderr, "There is something seriously wrong with your assignment of head/tail to the list\n");
      free(p);
      return NULL;
    }
  else
    {
      /* printf("List not empty, adding element to tail\n"); */
      s->tail->next = p;
      s->tail = p;
    }

  return s;
}


/* Remove the first element */
void* dequeue( struct queue* s )
{
  struct my_struct* h = NULL;
  struct my_struct* p = NULL;
  void * ret;

  if( NULL == s )
    {
      //printf("List is empty\n");
      return NULL;
    }
  else if( NULL == s->head && NULL == s->tail )
    {
      //printf("Well, List is empty\n");
      return NULL;
    }
  else if( NULL == s->head || NULL == s->tail )
    {
      printf("There is something seriously wrong with your list\n");
      printf("One of the head/tail is empty while other is not \n");
      return NULL;
    }

  h = s->head;
  p = h->next;
  ret = h->data;
  free(h);
  s->head = p;
  if( NULL == s->head )  s->tail = s->head;   /* The element tail was pointing to is free(), so we need an update */

  return ret;
}

/* Search an element and remove it from queue if found */
void* queue_find_remove(struct queue* s, void * data )
{
  void * ret;
 
 if( NULL == s )
   {
     //printf("List is empty\n");
     return NULL;
   }
 else 
   if( NULL == s->head && NULL == s->tail )
     {
       //printf("Well, List is empty\n");
       return NULL;
     }
   else if( NULL == s->head || NULL == s->tail )
     {
       printf("There is something seriously wrong with your list\n");
       printf("One of the head/tail is empty while other is not \n");
       return NULL;
     }
 
 if ( s->head->data == data) {
   ret = data;
   if (s->head == s->tail){
     free(s->head);
     s->head = s->tail = NULL;
   }
   else {
     struct my_struct* aux = s->head;
     s->head = s->head->next;
     free(aux);
   }
   return ret; 
 }
 else {
   struct my_struct* aux;
   
   for ( aux = s->head; aux->next && (aux->next->data != data); aux = aux->next);
   if (aux->next == NULL)
     return NULL;
   else {
     struct my_struct* aux2 =  aux->next;
     ret = aux->next->data; 
     if (aux->next->next == NULL )  // last element contains the searched data
       s->tail = aux;
     aux->next = aux->next->next;
     free(aux2);
     return ret;
   }
 } 
}

int queue_empty ( struct queue* s ) { return (s->head == NULL); }

/* ---------------------- small helper fucntions ---------------------------------- */
struct queue* queue_free( struct queue* s )
{
  while( s->head )
    {
      dequeue(s);
    }

  return s;
}

struct queue* queue_new(void)
{
  struct queue* p = malloc( 1 * sizeof(*p));

  if( NULL == p )
    {
      fprintf(stderr, "LINE: %d, malloc() failed\n", __LINE__);
    }

  p->head = p->tail = NULL;
  
  return p;
}


void queue_print(struct queue* ps )
{
  struct my_struct* p = NULL;
  printf("Queue contents:\n");
  if( ps )
    {
      if (queue_empty(ps))
	printf("\t\tEmpty QUEUE\n");
      else
	for( p = ps->head; p; p = p->next )
	  queue_print_element(p);
    }
}

void queue_print_element(struct my_struct* p )
{
  if( p ) 
    {
      struct  aio_rq *a = (struct aio_rq *)p->data;
      printf("\t\tThread ID = %d  Type = %s Offset = %d \n", a->tid, (a->type==READ_RQ)?"Read":"Write", a->offset);
    }
  else
    {
      printf("Can not print NULL struct \n");
    }
}


 /*
int main(void)
{
  struct queue*  mt = NULL;

  mt = queue_new();
  enqueue(mt, (void *)1);
  enqueue(mt, (void *)2);
  enqueue(mt, (void *)3);
  enqueue(mt, (void *)4); 
  
  queue_print(mt);

  dequeue(mt);
  queue_print(mt);

  queue_free(mt);   // always remember to free() the malloc()ed memory 
free(mt);        // free() if list is kept separate from free()ing the structure, I think its a good design 
mt = NULL;      // after free() always set that pointer to NULL, C will run havon on you if you try to use a dangling pointer 

  queue_print(mt);

  return 0;
}
*/

