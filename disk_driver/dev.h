#ifndef _DEV_H_
#define _DEV_H_
#include <pthread.h>
#include "queue.h"

#define DEVICE_SECTOR_SIZE 256	/* In bytes */
#define MAXDEVICES 10
#define FREE 0
#define INIT 1

#define READ_RQ 0
#define WRITE_RQ 1

typedef struct device{
  int state; /* FREE or INIT*/
  int fd; /* file descriptor of the file simulating the disk */
  int size; /* total size in bytes */
} devattr;

struct aio_rq{
  int dd; /* device descriptor */
  int tid; /* thread id */
  int offset; /* disk offset */
  int type;  /* type =   READ_RQ/WRIYE_RQ */
  char *buffer; /* buffer to be read or written */
};
  

/*  Returns total number of device sectors or a -1 for errors. */

int getNumSectors(int dd);

/* Reads into buffer one disk sector starting from a device offset 
 * The device offset is expressed in number of blocks
 * Returns the number of read sectors or a negative number in case of errors
 */

int dev_read(int dd, char *buffer, int offset );

/* Writes from buffer one sector starting from a device offset
 * The device offset is expressed in number of blocks
 * Returns the number of written sectors or a negative number in case of errors
 */

int dev_write(int dd, char *buffer,  int offset );

/* Initializes  a device: name is the file simulating the block device 
 * Returns a device descriptor or -1 in case of errors. 
*/
int dev_open(char *name);

/* Releases a device. It waits that all operations on device are finished. 
 */
int dev_rls(int dd);


/* Starts a read operation. 
   Returns 0 if successful and -1 for errors */ 
int async_read(struct aio_rq *r);
/* Starts a write operation. 
   Returns 0 if successful and -1 for errors */ 
int async_write(struct aio_rq *r);
/*  Checks the status of the r request. 
Returns 1 if operation has finished, 0 otherwise 
*/
int async_status(struct aio_rq *r);
/*  Blocks and waits until the current operation finishes.
 */
int async_wait(struct aio_rq *r);
/* Cancel an already started operations.
   If the operation has been already scheduled it waits to finish  
 */
int async_cancel(struct aio_rq *r);

#endif
