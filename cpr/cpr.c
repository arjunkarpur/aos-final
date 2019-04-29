#define _GNU_SOURCE

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <libaio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

static bool copy_internal(char const *src_name, char const *dst_name);
static bool copy_dir(char const *src_name, char const *dst_name);
static bool copy_reg(char const *src_name, char const *dst_name);
static int aio_copy_p(char const *src_name, char const *dst_name);

int main(int argc, char **argv) {
  copy_internal("poop", "poop");
}

bool copy_internal(char const *src_name, char const *dst_name) {
  struct stat src_sb;
  struct stat dst_sb;

  if ((stat(src_name, &src_sb) == -1) || (stat(dst_name, &dst_sb) == -1)){
    perror("stat failed");
    return 0;
  }

  if (S_ISDIR(src_sb.st_mode)) {
    assert(!S_ISDIR(dst_sb.st_mode));

    if (mkdir(dst_name, S_IRUSR | S_IWUSR) != 0) {
      perror("mkdir failed");
    }

    return copy_dir(src_name, dst_name);
  } else if (S_ISREG(src_sb.st_mode)) {
    return copy_reg(src_name, dst_name);
  }

  return 0;
}

bool copy_dir(char const *src_name_in, char const *dst_name_in) {
  // TODO(Paul)
  return 0;
}

bool copy_reg(char const *src_name, char const *dst_name) {
  // Create new thread and have it execute aio_copy_p
  printf("copy %s to %s\n", src_name, dst_name);
  if (fork() == 0) {
    int ret = aio_copy_p(src_name, dst_name);
    if (ret != 0) {
        //TODO: handle child process failure
    }
    exit(0);
  }
  return 0;
}

int aio_copy_p(char const *src_name, char const *dst_name) {
  // Perform copy of single file using AIO

  // Create io_context
  io_context_t aio_context = 0;
  if (io_setup(100, &aio_context))
  {
    perror("Failed to create aio context...");
    return -1;
  }

  // Get src file size and create buffer
  int src_fd = open(src_name, O_DIRECT, "rb");
  if (src_fd < 0)
  {
    perror("Failed to open src file...");
    return -1;
  }
  struct stat f_stat;
  if (fstat(src_fd, &f_stat) != 0)
  {
    perror("Failed to get src file size...");
    return -1;
  }
  int fsize = f_stat.st_size;
  char *buffer = mmap(NULL, fsize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (!buffer)
  {
    perror("Failed to mmap buffer...");
    return -1;
  }

  // Submit read request for src file
  struct iocb read_req;
  struct iocb *read_iocbs = &read_req;
  io_prep_pread(&read_req, src_fd, buffer, fsize, 0);
  if (io_submit(aio_context, 1, &read_iocbs) != 1)
  {
    perror("Failed to submit src file read request...");
    return -1;
  }

  // Spin until read completes
  struct io_event read_events[1];
  while (io_getevents(aio_context, 0, 1, &read_events[0], NULL) == 0) {} //spin
  if (read_events[0].res < 0)
  {
    perror("Failed to read src file...");
    return -1;
  }
  close(src_fd);

  // Submit write request to dest file
  int dest_fd = open(dst_name, O_RDWR | O_CREAT | O_DIRECT, 0666);
  if (dest_fd < 0)
  {
    perror("Failed to open dest file...");
    return -1;
  }
  struct iocb write_req;
  struct iocb *write_iocbs = &write_req;
  io_prep_pwrite(&write_req, dest_fd, buffer, fsize, 0);
  if (io_submit(aio_context, 1, &write_iocbs) != 1)
  {
    perror("Failed to submit dest file write request...");
    return -1;
  }

  // Spin until write completes, then clean up
  struct io_event write_events[1];
  while (io_getevents(aio_context, 0, 1, &write_events[0], NULL) == 0) {} //spin
  if (write_events[0].res < 0)
  {
    perror("Failed to write dest file...");
    return -1;
  }
  if (io_destroy(aio_context) != 0)
  {
    perror("Failed to destroy aio_context");
    return -1;
  }
  return 0;
}
