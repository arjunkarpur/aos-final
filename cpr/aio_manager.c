#include "aio_manager.h"
#include <stdlib.h>

extern volatile int req_count;

int init_aio_manager(aio_manager_t *aio_manager, int max_events, int read_batch_size, int write_batch_size) {

  // Initialize all fields of aio_manager
  aio_manager->aio_context = 0;
  if (io_setup(max_events, &aio_manager->aio_context)) {
    return -1;
  }
  aio_manager->read_head = NULL;
  aio_manager->write_head = NULL;
  aio_manager->request_count = 0;
  aio_manager->read_len = 0;
  aio_manager->write_len = 0;
  aio_manager->finished_count = 0;
  aio_manager->flush = false;
  aio_manager->read_batch_size = read_batch_size;
  aio_manager->write_batch_size = write_batch_size;
  if (pthread_mutex_init(&aio_manager->request_mutex, NULL)) {
    return -1;
  }
  if (pthread_mutex_init(&aio_manager->flush_mutex, NULL)) {
    return -1;
  }
  return 0;
}

int start_aio_thread(aio_manager_t *aio_manager) {
  return pthread_create(&aio_manager->aio_thread, NULL, aio_loop, aio_manager);
}

void *aio_loop(void *vargp) {
  // Perform aio reads and writes until finished
  aio_manager_t *aio_manager = vargp;
  while (true) {

    // Lock flush lock and request lock
    pthread_mutex_lock(&aio_manager->flush_mutex);

    // Check for any results and handle
    struct io_event events[10];
    int num_ret = io_getevents(aio_manager->aio_context, 0, 10, (struct io_event *)&events, NULL);
    for (int i = 0; i < num_ret; i++) {
        struct io_event *curr_event = &events[i];
        copy_request_t *copy_req = (struct copy_request_t *)curr_event->data;
        if (curr_event->res != copy_req->fsize) {
          perror("READ/WRITE FAILED\n");
          exit(1);
        }

        if (copy_req->state == 1) {
          // Handle the finished read
          close(copy_req->src_fd);
          copy_req->next = aio_manager->write_head;
          aio_manager->write_head = copy_req;
          aio_manager->write_len += 1;
        } else if (copy_req->state == 2) {
          // Handle the finished write
          close(copy_req->dst_fd);
          aio_manager->finished_count += 1;
          free(copy_req->src_name);
          free(copy_req->dst_name);
          free(copy_req);
        }
        free(curr_event->obj);
    }

    // Determine if finished
    //   No need to get request lock. If flush is true, then
    //   other thread is finished submitting copy requests
    if (aio_manager->flush && (req_count == aio_manager->finished_count)) {
      /* printf("req_count = %d\n", req_count); */
      /* printf("request_count = %d, finished_count = %d\n", */
      /*     aio_manager->request_count, aio_manager->finished_count); */
      pthread_mutex_unlock(&aio_manager->flush_mutex);
      break;
    }

    // Submit aio read requests, if necessary
    pthread_mutex_lock(&aio_manager->request_mutex);
    int read_size = 0;
    if (aio_manager->flush) {
      read_size = aio_manager->read_len;
    } else if (aio_manager->read_len >= aio_manager->read_batch_size) {
      read_size = aio_manager->read_batch_size;
    }
    if (read_size > 0) {
      // Submit read requests for first 'read_size' elements in read_head LL
      struct iocb *read_iocbs[read_size];
      for (int i=0; i < read_size; i++) {
        copy_request_t *curr = aio_manager->read_head;
        curr->src_fd = open(curr->src_name, O_DIRECT, "rb");
        if (curr->src_fd < 0) {
          perror("READ - FAILED TO OPEN SRC FILE\n");
          exit(1);
        }
        struct stat f_stat;
        if (fstat(curr->src_fd, &f_stat)) {
          perror("READ - FAILED TO FSTAT SRC FILE\n");
          exit(1);
        }
        curr->fsize = f_stat.st_size;
        curr->buffer = mmap(NULL, curr->fsize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (!curr->buffer) {
          perror("READ - FAILED TO ALLOCATE BUFFER\n");
          exit(1);
        }

        // Create iocb
        struct iocb *read_req = malloc(sizeof(struct iocb));
        io_prep_pread(read_req, curr->src_fd, curr->buffer, curr->fsize, 0);
        read_req->data = (void *)curr;
        read_iocbs[i] = read_req;

        // LL updates
        aio_manager->read_head = aio_manager->read_head->next;
        aio_manager->read_len -= 1;
        curr->next = NULL;
        curr->state = 1;
      }
      // Submit read requests
      if (io_submit(aio_manager->aio_context, read_size, (struct iocb **)&read_iocbs) != read_size) {
        perror("READ - FAILED TO IO_SUBMIT ALL REQUESTS\n");
        exit(1);
      }
    }
    pthread_mutex_unlock(&aio_manager->request_mutex);

    // Submit aio write requests, if necessary
    int write_size = 0;
    if (aio_manager->flush) {
      write_size = aio_manager->write_len;
    } else if (aio_manager->write_len >= aio_manager->write_batch_size) {
      write_size = aio_manager->write_batch_size;
    }
    if (write_size > 0) {
      struct iocb *write_iocbs[write_size];
      for (int i = 0; i < write_size; i++) {
        // Open dest file
        copy_request_t *curr = aio_manager->write_head;
        curr->dst_fd = open(curr->dst_name, O_RDWR | O_CREAT | O_DIRECT, 0666);
        if (curr->dst_fd < 0) {
          perror("WRITE - FAILED TO OPEN DST FILE\n");
          exit(1);
        }

        // Create iocb
        struct iocb *write_req = malloc(sizeof(struct iocb));
        io_prep_pwrite(write_req, curr->dst_fd, curr->buffer, curr->fsize, 0);
        write_req->data = (void *)curr;
        write_iocbs[i] = write_req;

        // LL updates
        aio_manager->write_head = aio_manager->write_head->next;
        aio_manager->write_len -= 1;
        curr->next = NULL;
        curr->state = 2;
      }
      // Submit write requests
      if (io_submit(aio_manager->aio_context, write_size, (struct iocb **) &write_iocbs) != write_size) {
        perror("WRITE - FAILED TO IO_SUBMIT ALL REQUESTS\n");
        exit(1);
      }
    }

    // Unlock flush lock
    pthread_mutex_unlock(&aio_manager->flush_mutex);
  }
  return NULL;
}

int add_copy_req(aio_manager_t *aio_manager, copy_request_t *copy_request) {
  // Atomically add to read LL
  if (pthread_mutex_lock(&aio_manager->request_mutex)) {
    return -1;
  }

  aio_manager->request_count += 1;
  aio_manager->read_len += 1;
  copy_request->next = aio_manager->read_head;
  aio_manager->read_head = copy_request;

  if (pthread_mutex_unlock(&aio_manager->request_mutex)) {
    return -1;
  }
  return 0;
}

int wait_for_aio_finish(aio_manager_t *aio_manager) {
  // Set flush bool to true
  if (pthread_mutex_lock(&aio_manager->flush_mutex)) {
    return -1;
  }
  aio_manager->flush = true;
  if (pthread_mutex_unlock(&aio_manager->flush_mutex)) {
    return -1;
  }

  // Wait for child pthread to finish
  if (pthread_join(aio_manager->aio_thread, NULL)) {
    return -1;
  }
  return 0;
}

int destroy_aio_manager(aio_manager_t *aio_manager) {
  // Kill aio_context
  if (io_destroy(aio_manager->aio_context)) {
    perror("Failed to destroy aio_context");
    return -1;
  }

  // Destroy mutexes
  if (pthread_mutex_destroy(&aio_manager->request_mutex)) {
    perror("Failed to destroy request_mutex");
    return -1;
  }
  if (pthread_mutex_destroy(&aio_manager->flush_mutex)) {
    perror("Failed to destroy flush_mutex");
    return -1;
  }
  return 0;
}
