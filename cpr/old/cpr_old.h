#ifndef CPR_H
#define CPR_H

#include <sys/queue.h>

typedef struct node {
    char *src_name;
    char *dst_name;
    TAILQ_ENTRY(node) nodes;
} node_t;

typedef TAILQ_HEAD(head_s, node) head_t;

static bool copy_internal(char const *src_name, char const *dst_name);
static bool copy_dir(char const *src_name, char const *dst_name, head_t *head);
static bool copy_reg(char const *src_name, char const *dst_name);
static int aio_copy_p(char const *src_name, char const *dst_name);

static int wait_for_children();

#endif
