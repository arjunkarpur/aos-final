#define _GNU_SOURCE

#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <libaio.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

static bool copy_internal(char const *src_name, char const *dst_name);
static bool copy_dir(char const *src_name, char const *dst_name);
static bool copy_reg(char const *src_name, char const *dst_name);
static int aio_copy_p(char const *src_name, char const *dst_name);

static int wait_for_children();
int MAX_EVENTS = 100;

int main(int argc, char **argv) {
  // Parse command line args
  if (argc != 3) {
      printf("Usage: ./cpr SRC_FP DEST_FP\n");
      return -1;
  }

  // Run copy routine on each file
  if (copy_internal(argv[1], argv[2]) != 0) {
    perror("copy failed");
    return -1;
  }

  // Wait for copies to finish and close
  if (wait_for_children() != 0) {
      return -1;
  }
}

int wait_for_children() {
  pid_t wpid;
  int status = 0;
  while ((wpid = wait(&status)) > 0);
  return 0;
}

bool copy_internal(char const *src_name, char const *dst_name) {
  struct stat src_sb;

  if (stat(src_name, &src_sb) == -1) {
    perror("stat failed");
    return 1;
  }

  if (S_ISDIR(src_sb.st_mode)) {
    if (mkdir(dst_name, 0777) != 0) {
      perror("mkdir failed");
      return 1;
    }

    return copy_dir(src_name, dst_name);
  } else if (S_ISREG(src_sb.st_mode)) {
    return copy_reg(src_name, dst_name);
  }

  return 0;
}

bool copy_dir(char const *src_name_in, char const *dst_name_in) {
  DIR *dir;
  if ((dir = opendir(src_name_in)) != NULL) {
    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
      if (strcmp(".", ent->d_name) == 0 || strcmp("..", ent->d_name) == 0) {
        continue;
      }
      if (ent->d_type == DT_DIR || ent->d_type == DT_REG) {
        char *src_name = malloc(strlen(src_name_in) + strlen(ent->d_name) + 1);
        char *dst_name = malloc(strlen(dst_name_in) + strlen(ent->d_name) + 1);
        strcpy(src_name, src_name_in);
        strcat(src_name, "/");
        strcat(src_name, ent->d_name);
        strcpy(dst_name, dst_name_in);
        strcat(dst_name, "/");
        strcat(dst_name, ent->d_name);
        int ret = copy_internal(src_name, dst_name);
        free(src_name);
        free(dst_name);
        return ret;
      }
    }
    closedir(dir);
  } else {
    perror("opendir failed");
    return 1;
  }
  return 0;
}

bool copy_reg(char const *src_name, char const *dst_name) {
  // Create new thread and have it execute aio_copy_p

  // Fork and copy
  if (fork() == 0) {
    if (aio_copy_p(src_name, dst_name) != 0) {
        //TODO: handle child process failure
    }
    exit(0);
  }
  return 0;
}

int aio_copy_p(char const *src_name, char const *dst_name) {
  // Perform copy of single file using AIO

  io_context_t aio_context = 0;
  if (io_setup(MAX_EVENTS, &aio_context)) {
    perror("Failed to create aio context...");
    return -1;
  }

  // Get src file size and create buffer
  int src_fd = open(src_name, O_DIRECT, "rb");
  if (src_fd < 0) {
    perror("Failed to open src file...");
    return -1;
  }
  struct stat f_stat;
  if (fstat(src_fd, &f_stat) != 0) {
    perror("Failed to get src file size...");
    return -1;
  }
  int fsize = f_stat.st_size;
  char *buffer = mmap(NULL, fsize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (!buffer) {
    perror("Failed to mmap buffer...");
    return -1;
  }

  // Submit read request for src file
  struct iocb read_req;
  struct iocb *read_iocbs = &read_req;
  io_prep_pread(&read_req, src_fd, buffer, fsize, 0);
  if (io_submit(aio_context, 1, &read_iocbs) != 1) {
    perror("Failed to submit src file read request...");
    return -1;
  }

  // Spin until read completes
  struct io_event read_events[1];
  while (io_getevents(aio_context, 0, 1, &read_events[0], NULL) == 0) {} //spin
  if (read_events[0].res < 0) {
    perror("Failed to read src file...");
    return -1;
  }
  close(src_fd);

  // Submit write request to dest file
  int dest_fd = open(dst_name, O_RDWR | O_CREAT | O_DIRECT, 0666);
  if (dest_fd < 0) {
    perror("Failed to open dest file...");
    return -1;
  }
  struct iocb write_req;
  struct iocb *write_iocbs = &write_req;
  io_prep_pwrite(&write_req, dest_fd, buffer, fsize, 0);
  if (io_submit(aio_context, 1, &write_iocbs) != 1) {
    perror("Failed to submit dest file write request...");
    return -1;
  }

  // Spin until write completes, then clean up
  struct io_event write_events[1];
  while (io_getevents(aio_context, 0, 1, &write_events[0], NULL) == 0) {} //spin
  if (write_events[0].res < 0) {
    perror("Failed to write dest file...");
    return -1;
  }

  // Destroy io_context
  if (io_destroy(aio_context) != 0) {
    perror("Failed to destroy aio_context");
    return -1;
  }
  return 0;
}
