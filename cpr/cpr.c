#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>

static bool copy_internal(char const *src_name, char const *dst_name);
static bool copy_dir(char const *src_name, char const *dst_name);
static bool copy_reg(char const *src_name, char const *dst_name);

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
  printf("copy %s to %s\n", src_name, dst_name);
  return 0;
}
