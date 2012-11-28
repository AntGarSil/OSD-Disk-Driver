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
Synchronously writes a sector to a device, reads it back and checks for correctness.  
 */

int main(int argc, char *argv[])
{
  int i, dd;
  for (i=0;i<DEVICE_SECTOR_SIZE;i++){
    buf1[i]='a';
    buf2[i]=0;
  }
  
   if ((dd=dev_open(DISKIMG)) < 0) 
    exit(1);
  
  if (dev_write(dd,buf1,0)< 0) 
    exit(1);

  if (dev_read(dd,buf2,0)< 0) 
    exit(1);

  if (memcmp(buf1, buf2,DEVICE_SECTOR_SIZE))
    printf("Read value different from writen value\n");
  
  if (dev_rls(dd) < 0) 
    exit(1);

  return 0;
} /****** End main() ******/


