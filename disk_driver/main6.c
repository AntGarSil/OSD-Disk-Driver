#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>
#include <string.h>

#include "dev.h"

#define DISKIMG "disk1.img"
#define NR_REQS 10


char buf1[DEVICE_SECTOR_SIZE];
/* 
 Asynchronously writes several sectors to a device.
 Overlaps computation with I/O
 Waits for all operations to finish. 
 */

int main(int argc, char *argv[])
{
  int i,j,k, dd;
  struct aio_rq *p[NR_REQS]; 
  for (i=0;i<NR_REQS;i++)
    p[i] = (struct aio_rq *)malloc (sizeof (struct aio_rq));
 

  for (i=0;i<DEVICE_SECTOR_SIZE;i++){
    buf1[i]='x';
  }
  
  if ((dd=dev_open(DISKIMG)) < 0) 
    exit(1);
  
  for (i=0;i<NR_REQS;i++){
    p[i]->dd = dd;
    p[i]->offset = i; /* ith sector */
    p[i]->buffer = buf1;
    p[i]->tid = pthread_self();
    
    if (async_write(p[i]) < 0) {
      printf("Error in async_write");
      exit(1);
    } 
  }

  printf("******Overlap some compution with I/O******\n");
  k=0;
  for (j=0;j<10*1000*1000;j++) k++;

  printf("Waiting for the I/O to finish\n");
  /* Blocking waiting for the I/O to finish */
  for (i=0;i<NR_REQS;i++)
    if (async_wait(p[i]) < 0) {
      printf("Error in async_write");
      exit(1);
    } 
  printf("Done\n");
  if (dev_rls(dd) < 0) 
    exit(1);

  return 0;
} /****** End main() ******/


