#include <stdlib.h>
#include <stdio.h>
#include <libaio.h>

int do_copy(char *src_fp, char *dest_fp)
{
    // Perform copy of single file using AIO

    // Create io_context
    int ret;
    io_context_t aio_context;
    ret = io_setup(1, &aio_context);

    // Submit read request for src file
    iocb read_reqs[1];
    int fsize = 1024;
    int src_fd = open(src_fp, O_DIRECT,"rb");
    char *buffer = malloc(fsize);
    io_prep_pread(&iocb[0], src_fd, buffer, fsize, 0);
    io_submit(aio_context, 1, &read_reqs);

    // Spin until read completes
    io_event read_events[1];
    while (io_getevents(aio_context, 0, 1, &read_events, NULL) == 0) {} //spin
    if (read_events[0].res < 0)
    {
        printf("Failed to read src file...EXITING\n");
        return -1;
    }
    close(src_fd);

    // Submit write request to dest file
    iocb write_reqs[1];
    int dest_fd = open(dest_fp, O_DIRECT, "ab");
    io_prep_write(&write_reqs[0], dest_fd, buffer, fsize, 0);
    io_submit(aio_context, 1, %write_reqs);

    // Spin until write completes, then clean up
    io_event write_events[1];
    while (io_getevents(aio_context, 0, 1, %write_events, NULL) == 0) {} //spin
    if (write_events[0].res < 0)
    {
        printf("Failed to write dest file...EXITING\n");
        return -1;
    }
    io_destroy(aio_context);
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
