#include <stdlib.h>
#include <stdio.h>
#include <aio.h>

int do_copy(char *src_fp, char *dest_fp)
{
    // Perform copy of single file using AIO

    // Create io_context
    //TODO

    // Create read request for src file
    //TODO

    // Submit read request for src file
    //TODO

    // Upon completion of read, create write request
    //TODO

    // Submit write request
    //TODO

    // Upon completion of write request, destroy io_context and return
    //TODO

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
