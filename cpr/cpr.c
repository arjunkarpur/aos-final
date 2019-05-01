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
#include <sys/queue.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "cpr.h"

int MAX_EVENTS = 100;

int main(int argc, char **argv) {
  // Parse command line args
  if (argc != 3) {
      printf("Usage: ./cpr SRC_FP DEST_FP\n");
      return -1;
  }

  // Run copy routine on each file
  if (copy_internal(argv[1], argv[2])) {
    perror("copy failed");
    return -1;
  }
}

bool copy_internal(char const *src_name, char const *dst_name) {
  head_t head;
  TAILQ_INIT(&head);

  node_t *node = malloc(sizeof(node_t));
  node->src_name = malloc(strlen(src_name));
  node->dst_name = malloc(strlen(dst_name));
  strcpy(node->src_name, src_name);
  strcpy(node->dst_name, dst_name);
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
    while ((ent = readdir(dir)) != NULL) {
      if (strcmp(".", ent->d_name) == 0 || strcmp("..", ent->d_name) == 0) {
        continue;
      }
      if (ent->d_type == DT_DIR || ent->d_type == DT_REG) {
        node_t *node = malloc(sizeof(node_t));
        node->src_name = malloc(strlen(src_name_in) + strlen(ent->d_name) + 1);
        node->dst_name = malloc(strlen(dst_name_in) + strlen(ent->d_name) + 1);
        strcpy(node->src_name, src_name_in);
        strcat(node->src_name, "/");
        strcat(node->src_name, ent->d_name);
        strcpy(node->dst_name, dst_name_in);
        strcat(node->dst_name, "/");
        strcat(node->dst_name, ent->d_name);
        TAILQ_INSERT_TAIL(head, node, nodes);
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
  //TODO: arjun
  return 0;
}
