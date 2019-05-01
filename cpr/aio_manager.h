#ifndef AIO_MANAGER_H
#define AIO_MANAGER_H

#define _GNU_SOURCE

#include <stdbool.h>
#include <pthread.h>
#include <libaio.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>

typedef struct copy_request_t {
  char *src_name;
  char *dst_name;
  int src_fd;
  int fsize;
  int dst_fd;
  char *buffer;
  struct copy_request_t *next;
} copy_request_t;

typedef struct aio_manager_t {
    
  // AIO context and thread
  io_context_t aio_context;
  pthread_t aio_thread;

  // Linked lists to manage I/O
  copy_request_t *read_head;
  copy_request_t *read_inflight_head;
  copy_request_t *write_head;
  copy_request_t *write_inflight_head;

  // Counters
  int request_count;
  int read_len;
  int write_len;
  int finished_count;

  // Flush bool
  bool flush;

  // Batch size metadata
  int read_batch_size;
  int write_batch_size; 

  // Locks
  pthread_mutex_t request_mutex;
  pthread_mutex_t flush_mutex;

} aio_manager_t;

int init_aio_manager(aio_manager_t *aio_manager, int max_events, int read_batch_size, int write_batch_size);
int start_aio_thread(aio_manager_t *aio_manager);
int add_copy_req(aio_manager_t *aio_manager, copy_request_t *copy_request);
int wait_for_aio_finish(aio_manager_t *aio_manager);
int destroy_aio_manager(aio_manager_t *aio_manager);

void *aio_loop(void *vargp);

#endif
