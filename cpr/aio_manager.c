#include "aio_manager.h"

int init_aio_manager(aio_manager_t *aio_manager, int max_events, int read_batch_size, int write_batch_size) {

  // Initialize all fields of aio_manager
  aio_manager->aio_context = 0;
  if (io_setup(max_events, &aio_manager->aio_context)) {
    return -1;
  }
  aio_manager->read_head = NULL;
  aio_manager->read_inflight_head = NULL;
  aio_manager->write_head = NULL;
  aio_manager->write_inflight_head = NULL;
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
    //TODO
    // - make sure you close fd as necessary
    // - update counters
    // - free unnecessary copy_requests
    //   - free src_name and dst_name within those

    // Determine if finished
    //   No need to get request lock. If flush is true, then
    //   other thread is finished submitting copy requests
    if (aio_manager->flush && (aio_manager->request_count == aio_manager->finished_count)) {
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
          //TODO: handle failure to open file
          printf("READ - FAILED TO OPEN SRC FILE\n");
        }
        struct stat f_stat;
        if (fstat(curr->src_fd, &f_stat)) {
          //TODO: handle failure to fstat
          printf("READ - FAILED TO FSTAT SRC FILE\n");
        }
        curr->fsize = f_stat.st_size;
        curr->buffer = mmap(NULL, curr->fsize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (!curr->buffer) {
          //TODO handle failure to allocate buffer
          printf("READ - FAILED TO ALLOCATE BUFFER\n");
        }

        // Create iocb
        struct iocb *read_req = malloc(sizeof(struct iocb));
        io_prep_pread(read_req, curr->src_fd, curr->buffer, curr->fsize, 0);
        read_req->data = (void *)curr;
        read_iocbs[i] = read_req;

        // LL updates
        aio_manager->read_head = aio_manager->read_head->next;
        aio_manager->read_len -= 1;
        curr->next = aio_manager->read_inflight_head;
        aio_manager->read_inflight_head = curr;
      }
      // Submit read requests
      if (io_submit(aio_manager->aio_context, read_size, (struct iocb **)&read_iocbs) != read_size) {
        //TODO: handle failure of submission
        printf("READ - FAILED TO IO_SUBMIT ALL REQUESTS\n");
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
        copy_request_t *curr = aio_manager->write_head;
        curr->dst_fd = open(curr->dst_name, O_RDWR | O_CREAT | O_DIRECT, 0666);
        if (curr->dst_fd < 0) {
          //TODO: handle failure to open file
          printf("WRITE - FAILED TO OPEN DST FILE\n");
        }
        struct iocb *write_req = malloc(sizeof(struct iocb));
        io_prep_pwrite(write_req, curr->dst_fd, curr->buffer, curr->fsize, 0);
        write_req->data = (void *)curr;
        write_iocbs[i] = write_req;
      }
      // Submit write requests
      if (io_submit(aio_manager->aio_context, write_size, (struct iocb **) &write_iocbs) != write_size) {
        //TODO: handle failure of submission
        printf("WRITE - FAILED TO IO_SUBMIT ALL REQUESTS\n");
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
