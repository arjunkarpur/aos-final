#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{

    if (argc < 3) {
        printf("./copy_test src dest\n");
        return -1;
    }

    // Get src file size
    FILE *src_f = fopen(argv[1], "rb");
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
    FILE *dest_f = fopen(argv[2], "a");
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
