/* 
 * File:   main8.c
 *
 * Created on April 10, 2012, 9:55 PM
 */

#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>
#include <string.h>

#include "dev.h"

#define DISKIMG "disk1.img"


char buf1[DEVICE_SECTOR_SIZE];
char buf2[DEVICE_SECTOR_SIZE];


int main(int argc, char *argv[])
{
  int dd;
int i=0;
  struct aio_rq *p;
struct aio_rq *p2;
  p = (struct aio_rq *)malloc (sizeof (struct aio_rq));
  p2 = (struct aio_rq *)malloc (sizeof (struct aio_rq));

  if ((dd=dev_open(DISKIMG)) < 0)
    exit(1);
   for (i=0;i<DEVICE_SECTOR_SIZE;i++){
    buf1[i]='b';
    buf2[i]=0;
  }
  p->dd = dd;
  p->offset = 0;
  p->buffer = buf1;
  p->tid = pthread_self();

  for(i = 0; i < 21; i++)
  {
      if (async_write(p) < 0) {
        printf("Error in async_write");
        exit(1);
      }
      if(i%3==0)
      {
          printf("Funky computation blocked %d\n",i);
          async_wait(p);
          //printf("Funky computation unblocked %d\n",i);
      }
      printf("Default comp %d\n",i);

  }
  i=0;

  if (dev_rls(dd) < 0)
    exit(1);

  return 0;
} /****** End main() ******/

