#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "dev.h"

char *buf;

int main(int argc, char *argv[])
{
  int fd, sz;
  if (argc != 3) {
    printf("Utilization: %s DISK_IMAGE_NAME NUMBER_OF_SECTORS\n", argv[0]);
    exit(1);
  }  

  sz = atoi(argv[2]);
  if (sz > 8192) {
    printf("Image too large: maximum number of blocks is 8192\n");
    exit(1);
  }
  if ((fd=creat(argv[1],0666)) < 0) {
    perror("Error creating disk image");
    exit(1);
  }
  
  buf = malloc(DEVICE_SECTOR_SIZE*sz);

  if (write(fd,buf,DEVICE_SECTOR_SIZE*sz) < 0) {
     perror("Error writing disk image");
     exit(1);
  }

  close(fd);

  return 0;
} /****** End main() ******/


