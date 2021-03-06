#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>
#include <string.h>

#include "dev.h"

#define DISKIMG "disk1.img"


char buf1[DEVICE_SECTOR_SIZE],buf2[DEVICE_SECTOR_SIZE];

/* 
 Asynchronously writes a sector to a device
 Overlaps computation with I/O
 Synchronously reads it back and checks for correctness.  
 */

int main(int argc, char *argv[])
{
  int i,j,k, dd;
  struct aio_rq *p; 
  p = (struct aio_rq *)malloc (sizeof (struct aio_rq));

  for (i=0;i<DEVICE_SECTOR_SIZE;i++){
    buf1[i]='b';
    buf2[i]=0;
  }
  
  if ((dd=dev_open(DISKIMG)) < 0) 
    exit(1);
  
  p->dd = dd;
  p->type = WRITE_RQ;
  p->offset = 0;
  p->buffer = buf1;
  p->tid = pthread_self();
  
  if (async_write(p) < 0) {
    printf("Error in async_write");
  } 

  i=0;
  while (1) {
    printf("*** Computation overlapping I/O iteration=%d\n", i++);
    for (j=0;j<1000*1000;j++) k++;
    printf("*** Checking if I/O terminated\n");
    if (async_status(p) > 0) {
      printf("\tI/O operation finished.\n");
      break;
    }
    else
      printf("\tI/O operation NOT finished yet.\n");
  }
      
  if (dev_read(dd,buf2,0)< 0) 
    exit(1);
  
  if (memcmp(buf1, buf2,DEVICE_SECTOR_SIZE))
    printf("Read value different from writen value\n");
  
  if (dev_rls(dd) < 0) 
    exit(1);

  return 0;
} /****** End main() ******/


