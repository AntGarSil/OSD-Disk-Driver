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
/* 
 Asynchronously writes a sector to a device
 Cancels the operation
 */

int main(int argc, char *argv[])
{
  int dd;
  struct aio_rq *p; 
  p = (struct aio_rq *)malloc (sizeof (struct aio_rq));

  if ((dd=dev_open(DISKIMG)) < 0) 
    exit(1);
  
  p->dd = dd;
  p->offset = 0;
  p->buffer = buf1;
  p->tid = pthread_self();
  
  if (async_write(p) < 0) {
    printf("Error in async_write");
    exit(1);
  } 

  /* Cancel write */
  if (async_cancel(p) < 0) {
    printf("Error in async_cancel");
    exit(1);
  } 
  
  if (dev_rls(dd) < 0) 
    exit(1);

  return 0;
} /****** End main() ******/


