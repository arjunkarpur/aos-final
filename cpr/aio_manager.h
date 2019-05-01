#ifndef AIO_MANAGER_H
#define AIO_MANAGER_H

#include <stdbool.h>
#include <pthread.h>
#include <libaio.h>

typedef struct copy_request_t {
    const char *src_name;
    const char *dst_name;
    char *buffer;
    struct copy_request_t *next;
} copy_request_t;

typedef struct aio_manager_t {
    
    // AIO context and thread
    io_context_t aio_context;
    pthread_t aio_thread;

    // Linked lists to manage I/O
    copy_request_t *read_head;
    copy_request_t *inflight_head;
    copy_request_t *write_head;

     // Counters
    int read_count;
    int write_count;
    int finished_count;

    // Flush bool
    bool flush;

    // Batch size metadata
    int read_batch_size;
    int write_batch_size; 

    // Locks
    pthread_mutex_t *read_mutex;
    pthread_mutex_t *flush_mutex;

} aio_manager_t;

int init_aio_manager(aio_manager_t *aio_manager);
int start_aio_thread(aio_manager_t *aio_manager);
int aio_loop(aio_manager_t *aio_manager);
int add_copy_req(aio_manager_t *aio_manager, copy_request_t *copy_request);
int wait_for_aio_finish(aio_manager_t *aio_manager);

#endif
