#define _GNU_SOURCE

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/queue.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "cpr.h"
#include "aio_manager.h"

// Params
int MAX_EVENTS = 65536;
int READ_BATCH_SIZE = 8;
int WRITE_BATCH_SIZE = 8;

volatile int req_count = 0;

aio_manager_t aio_manager;

int main(int argc, char **argv) {
  // Parse command line args
  if (argc != 3 && argc != 4) {
    printf("Usage: ./cpr SRC_FP DEST_FP [BATCH_SIZE]\n");
    return -1;
  } else if (argc == 4) {
    READ_BATCH_SIZE = atoi(argv[3]);
    WRITE_BATCH_SIZE = atoi(argv[3]);
  }

  // Init aio_manager thread
  if (init_aio_manager(&aio_manager, MAX_EVENTS, READ_BATCH_SIZE, WRITE_BATCH_SIZE)) {
    perror("Failed to init aio_manager");
    return -1;
  }
  if (start_aio_thread(&aio_manager)) {
    perror("Failed to start aio thread");
    return -1;
  }

  // Run copy routine on each file
  if (copy_internal(argv[1], argv[2])) {
    perror("copy failed");
    return -1;
  }

  /* printf("made %d requests\n", req_count); */
  // Wait for aio thread to finish all tasks
  if (wait_for_aio_finish(&aio_manager)) {
    perror("Failed to wait for aio to finish");
    return -1;
  }
  if (destroy_aio_manager(&aio_manager)) {
      perror("Failed to destroy aio_manager");
      return -1;
  }
  return 0;
}

bool copy_internal(char const *src_name, char const *dst_name) {
  head_t head;
  TAILQ_INIT(&head);

  node_t *node = malloc(sizeof(node_t));
  node->src_name = strdup(src_name);
  node->dst_name = strdup(dst_name);
  TAILQ_INSERT_TAIL(&head, node, nodes);

  while (!TAILQ_EMPTY(&head)) {
    node_t *curr = TAILQ_FIRST(&head);
    struct stat src_sb;

    if (stat(curr->src_name, &src_sb) == -1) {
      perror("stat failed");
      return 1;
    }

    if (S_ISDIR(src_sb.st_mode)) {
      if (mkdir(curr->dst_name, 0777)) {
        perror("mkdir failed");
        return 1;
      }

      if (copy_dir(curr->src_name, curr->dst_name, &head)) {
        return 1;
      }
    } else if (S_ISREG(src_sb.st_mode)) {
      req_count += 1;
      if (copy_reg(curr->src_name, curr->dst_name)) {
        return 1;
      }
    }

    free(curr->src_name);
    free(curr->dst_name);
    free(curr);
    TAILQ_REMOVE(&head, curr, nodes);
  }

  return 0;
}

bool copy_dir(char const *src_name_in, char const *dst_name_in, head_t *head) {
  DIR *dir;
  if ((dir = opendir(src_name_in)) != NULL) {
    struct dirent *ent;
    while (1) {
      errno = 0;
      ent = readdir(dir);
      if (ent == NULL) {
        if (errno) {
          perror("readdir failed");
          return 1;
        }
        break;
      }
      if (strcmp(".", ent->d_name) == 0 || strcmp("..", ent->d_name) == 0) {
        continue;
      }
      if (ent->d_type == DT_DIR || ent->d_type == DT_REG || ent->d_type == DT_UNKNOWN) {
        node_t *node = malloc(sizeof(node_t));
        node->src_name = malloc(strlen(src_name_in) + strlen(ent->d_name) + 2);
        node->dst_name = malloc(strlen(dst_name_in) + strlen(ent->d_name) + 2);
        strcpy(node->src_name, src_name_in);
        strcat(node->src_name, "/");
        strcat(node->src_name, ent->d_name);
        strcpy(node->dst_name, dst_name_in);
        strcat(node->dst_name, "/");
        strcat(node->dst_name, ent->d_name);
        TAILQ_INSERT_TAIL(head, node, nodes);
      } else {
        switch(ent->d_type) {
          case DT_BLK:
            printf("got DT_BLK\n"); exit(1);
          case DT_CHR:
            printf("DT_CHR\n"); exit(1);
          case DT_FIFO:
            printf("DT_FIFO\n"); exit(1);
          case DT_LNK:
            printf("DT_LNK\n"); exit(1);
          case DT_SOCK:
            printf("DT_SOCK\n"); exit(1);
          case DT_UNKNOWN:
            printf("DT_UNKNOWN\n"); exit(1);
          default:
            printf("what\n");
            exit(1);
        }
      }
    }
    if (closedir(dir)) {
      perror("closedir failed");
      return 1;
    }
  } else {
    perror("opendir failed");
    return 1;
  }
  return 0;
}

bool copy_reg(char const *src_name, char const *dst_name) {
  copy_request_t *copy_request = malloc(sizeof *copy_request);
  copy_request->src_name = strdup(src_name);
  copy_request->dst_name = strdup(dst_name);
  copy_request->src_fd = -1;
  copy_request->dst_fd = -1;
  copy_request->fsize = 0;
  copy_request->buffer = NULL;
  copy_request->state = 0;
  copy_request->next = NULL;
  if (add_copy_req(&aio_manager, copy_request)) {
      perror("Failed to add copy request to aio_manager");
      return 1;
  }
  return 0;
}
