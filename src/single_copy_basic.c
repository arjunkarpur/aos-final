#include <stdlib.h>
#include <stdio.h>

int do_copy(char *src_fp, char *dest_fp)
{
    // Get src file size
    FILE *src_f = fopen(src_fp, "rb");
    if (!src_f)
    {
        printf("SRC FILE DOESNT EXIST");
        return -1;
    }
    fseek(src_f, 0, SEEK_END);
    long fsize = ftell(src_f);
    fseek(src_f, 0, SEEK_SET);

    // Read into buffer
    char *buffer = malloc(fsize+1);
    fread(buffer, 1, fsize, src_f);
    fclose(src_f);

    // Create new file
    FILE *dest_f = fopen(dest_fp, "a");
    if (!dest_f)
    {
        printf("DEST FILE DOESNT EXIST");
        return -1;
    }

    // Write from buffer to new file
    fwrite(buffer, 1, fsize, dest_f);
    fclose(dest_f);
    return 0;
}


int main(int argc, char *argv[])
{
    // Get command line args
    if (argc < 3) {
        printf("./copy_test src dest\n");
        return -1;
    }

    // Perform copy
    int ret = do_copy(argv[1], argv[2]);
    return ret;
}
