#include "dev.h"
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <pthread.h>

#include "dev.h"
#include "queue.h"

pthread_t schedId;
pthread_t currentThread;
pthread_t blockedThread;
pthread_cond_t async_wait_sig;
pthread_cond_t scheduler_block_sig;
pthread_mutex_t async_wait_mutex;
pthread_mutex_t queue_mutex;
pthread_mutex_t petition_mutex;
pthread_mutex_t schedblock_mutex;
struct queue *initqueue;
struct queue *finishqueue;
struct aio_rq *currentReq;
struct aio_rq *blockedReq;
devattr device_table[MAXDEVICES];
pthread_attr_t attr;
int schedblock;


static int init=0;
//void *scheduler();

void dev_petition(void * ptr)
{
    pthread_mutex_lock(&petition_mutex);
    struct aio_rq *req = (struct aio_rq *) ptr;
    pthread_mutex_unlock(&petition_mutex);
    
    int dd = req->dd;
    char *buffer = req->buffer;
    int offset = req->offset;

    //sleep(5);
    //printf("dev_petition: Serving request\n");
    /// Filter Petitions according to their nature ///////////////
    if(req->type == READ_RQ) dev_read(dd,buffer,offset);
    if(req->type == WRITE_RQ) dev_write(dd,buffer,offset);
    /////////////////////////////////////////////////////////////

    //pthread_mutex_lock(&async_wait_mutex);
    if(blockedReq == req)pthread_cond_signal(&async_wait_sig);
    //pthread_cond_signal(&async_wait_sig);
    //pthread_mutex_unlock(&async_wait_mutex);

    pthread_mutex_lock(&queue_mutex);
    enqueue(finishqueue,currentReq);
    currentThread = 0;
    pthread_mutex_unlock(&queue_mutex);
    printf("dev_petition: Finished IO petition tid:%d offset:%d\n",currentReq->tid,currentReq->offset);
    pthread_exit(NULL);
}

void scheduler()
{
    while(1)
    {
        if(initqueue->head != NULL)
        {
            pthread_mutex_lock(&schedblock_mutex);
            schedblock = 0;
            pthread_mutex_unlock(&schedblock_mutex);
            ///////// Init queue has new request, create new petition thread //////////////////
            pthread_mutex_lock(&queue_mutex);
            //if(currentThread != 0) enqueue(finishqueue,currentReq); // Enqueue in finishqueue the finished request

            currentReq = (struct aio_rq *)dequeue(initqueue); //Obtain new request from ready queue

            currentThread = currentReq->tid; //Store thread id in corresponding datatype           
            pthread_mutex_unlock(&queue_mutex);
            
            pthread_create(&currentThread,NULL,(void *) dev_petition, currentReq);

            
            pthread_join(currentThread,NULL);
            currentReq = NULL;
            /////////////////////////////////////////////////////////////////
        }
        else
        {
            /////////// Block scheduler: No busy waiting /////////////////////
            pthread_mutex_lock(&schedblock_mutex);
            schedblock = 1;
            pthread_mutex_unlock(&schedblock_mutex);
            pthread_cond_wait(&scheduler_block_sig,&schedblock_mutex);
            pthread_mutex_unlock(&schedblock_mutex);
            /////////////////////////////////////////////////////////////////
        }
    }
}

int async_wait(struct aio_rq *req)
{

    	pthread_mutex_lock(&async_wait_mutex); //Wait must be an atomic operation

	if(req != currentReq){
            pthread_mutex_lock(&queue_mutex);
            struct aio_rq *found = queue_find_remove(finishqueue,req); //Check if req is in finishqueue
            pthread_mutex_unlock(&queue_mutex);
            if( found == NULL ){
                //////////// If input request is not in finish queue, block  ///////////
                blockedReq = req;
                ///////////////////////////////////////////////////////////////////////
            }
            else
            {
                //////////// Input request is in finish queue, do nothing ///////////////
                pthread_mutex_lock(&queue_mutex);
                enqueue(finishqueue, found);
                pthread_mutex_unlock(&queue_mutex);
                pthread_mutex_unlock(&async_wait_mutex);
                printf("async_wait: Request tid:%d offset:%d found in finish queue\n",found->tid,found->offset);
                return 0;
                ///////////////////////////////////////////////////////////////////////////
            }
	}
	if(req == currentReq){
            /////////////If the request is the same as the current IO, Block /////////////
            blockedReq = currentReq;
            ///////////////////////////////////////////////////////////////////////////
	}

        printf("async_wait: Blocking %d\n", blockedReq->tid); //Print to test blocking
        //pthread_mutex_unlock(&async_wait_mutex);
	pthread_cond_wait(&async_wait_sig, &async_wait_mutex); /// W8 for current IO to finish
        printf("async_wait: Unblocking %d\n",blockedReq->tid); //Print to test blocking
	blockedThread = 0;
	pthread_mutex_unlock(&async_wait_mutex);

	return 1;

}

int async_cancel(struct aio_rq *req)
{
    struct aio_rq *found = queue_find_remove(initqueue,req);
    if(found == NULL)
    {
        printf("async_cancel: Request not in ready queue\n");
        if(req == currentReq)
        {
            printf("async_cancel: Current petition taking place");
            async_wait(req);
            return 1;
        }
        return 0;
    }
    printf("async_cancel: Petition found and cancelled\n");
    return 1;
}


int async_write(struct aio_rq *req)
{
    
    if(enqueue(initqueue,req) == NULL) return -1;
    pthread_mutex_lock(&schedblock_mutex);
    if(schedblock == 1) pthread_cond_signal(&scheduler_block_sig);
    pthread_mutex_unlock(&schedblock_mutex);
    return 0;
}

int async_read(struct aio_rq *req)
{
    if(enqueue(initqueue,req) == NULL) return -1;
    pthread_mutex_lock(&schedblock_mutex);
    if(schedblock == 1) pthread_cond_signal(&scheduler_block_sig);
    pthread_mutex_unlock(&schedblock_mutex);
    return 0;
}

int async_status(struct aio_rq *req)
{
    struct aio_rq *found = queue_find_remove(finishqueue,req);

    int status = 0;
    if(found != NULL)
    {
        status = 1;
        enqueue(finishqueue,found);
    }

    return status;

}

void init_devices(){
  int i;
  for (i=0;i<MAXDEVICES;i++)
    device_table[i].state = FREE;
}


void init_drivers() {
  init_devices(); 
}


/* Initializes  a device: name is the file simulating the block device */
int dev_open(char *name){

  int fd,i;

  if (!init) {
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    initqueue = queue_new();
    finishqueue = queue_new();
    pthread_create(&schedId,&attr,(void *)scheduler,NULL);
    init_drivers(); 
    init=1;
  }

  if (name == NULL) {
    fprintf(stderr, "Error: The device name is NULL\n");
    return -2;
  }

  fd = open(name, O_RDWR|O_SYNC);
  if (fd < 0) {
    if (errno == ENOENT)
      printf("Could not open file %s \n\t !! Create the disk image with the create_disk_image utility !!\n",name);
    else
      perror("Could not open file");
    return -3;
  }
   
  for (i=0;i<MAXDEVICES;i++) 
    if (device_table[i].state == FREE){
      device_table[i].state = INIT;
      device_table[i].fd = fd;
      break;
    }

  if (i == MAXDEVICES) {
    fprintf(stderr, "Error: The device table is full \n");
    return -1;
  }
  
  return i;
}

/* Releases a device. It waits that all operations on device are finished. 
 */
int dev_rls(int dd) {
  int ret;
 
  if (!init) {
    printf("Error: devices have not been intialized");
    return -1;
  }

  if ( device_table[dd].state == FREE) {
    printf("Error: device has not been open");
    return -1;
  }

  device_table[dd].state = INIT;	
  ret = close(device_table[dd].fd);
  
  if (ret < 0) {
    perror("Could not close file:");
    return -2;
  }
  
  return 0;
}

/*  Returns total number of device sectors or a -1 for errors. */

int getNumSectors(int dd) {
  struct stat st;
 
  if (!init) {
    printf("Error: devices have not been intialized");
    return -1;
  }

  if ( device_table[dd].state == FREE) {
    printf("Error: device has not been open");
    return -1;
  } 
  if (fstat(device_table[dd].fd, &st) < 0) {
    perror("fstar error");
    return -2;
  }
  
  return st.st_size / DEVICE_SECTOR_SIZE;
}

/* Reads into buffer one sector starting from a device offset
 * Returns the number of read sectors or a negative number in case of errors
 */

int dev_read(int dd, char *buffer, int offset ) {
  int nread;
  
  if (!init) {
    printf("Error: devices have not been intialized");
    return -1;
  }

  if ( device_table[dd].state == FREE) {
    printf("Error: device has not been open");
    return -1;
  }
  	
  if (offset < 0) {
    fprintf(stderr, "Error: Negative offset \n");
    return -2;
  }
	
  if (buffer == NULL) {
    fprintf(stderr, "Error: buffer is NULL\n");
    return -3;
  }
	
 	
  if (lseek(device_table[dd].fd, offset * DEVICE_SECTOR_SIZE, SEEK_SET) < 0) {
    perror("lseek");
    return -5;
  }
	
  nread = read(device_table[dd].fd, buffer, DEVICE_SECTOR_SIZE);
  if (nread <= 0) {
    fprintf(stderr, "Error: Could not read from device \n");
    return -6;
  } 
  else 
    if (nread < DEVICE_SECTOR_SIZE) {
      fprintf(stderr, "Warning: Could not read all requested sectors from device\n");
    }
  
  return nread / DEVICE_SECTOR_SIZE;
}

/* Writes from buffer one sector starting from a device offset
 * Returns the number of written sectors or a negative number in case of errors
 */

int dev_write(int dd, char *buffer,  int offset ) {
  int nwrite;

  if (!init) {
    printf("Error: devices have not been intialized");
    return -1;
  }

  if ( device_table[dd].state == FREE) {
    printf("Error: device has not been open");
    return -1;
  }	

  if (offset < 0) {
    fprintf(stderr, "Error: Negative offset\n");
    return -2;
  }
  
  if (buffer == NULL) {
    fprintf(stderr, "Error: buffer is NULL\n");
    return -3;
  }
  
  if (lseek(device_table[dd].fd, offset * DEVICE_SECTOR_SIZE, SEEK_SET) < 0) {
    perror("lseek");
    return -5;
  }
  
  nwrite = write(device_table[dd].fd, buffer, DEVICE_SECTOR_SIZE);
  if (nwrite <= 0) {
    fprintf(stderr, "Error: Could not write to device\n");
    return -6;
  } else if (nwrite < DEVICE_SECTOR_SIZE) {
    fprintf(stderr, "Warning: Could not write all sectors to device \n");
  }
  
  return nwrite / DEVICE_SECTOR_SIZE;
}

