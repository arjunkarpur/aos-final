#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <libaio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int do_copy(char *src_fp, char *dest_fp)
{
    // Perform copy of single file using AIO

    // Create io_context
    int ret;
    io_context_t aio_context;
    ret = io_setup(1, &aio_context);

    // Get src file size and create buffer
    int src_fd = open(src_fp, O_DIRECT, "rb");
    if (src_fd < 0)
    {
        printf("Failed to open src file...EXITING\n");
        return -1;
    }
    int fsize = 1024;
    char *buffer = malloc(fsize);

    /*
    //TODO: why does this cause submission of read req to fail?
    //TODO:   see notes about aligning to pages when using O_DIRECT
    int test[23];
    int test2[23];

    struct stat f_stat;
    if (fstat(src_fd, &f_stat) != 0)
    {
        printf("Failed to get src file size...EXITING\n");
        return -1;
    }
    int fsize = f_stat.st_size;
    */

    // Submit read request for src file
    struct iocb read_req;
    struct iocb *read_iocbs = &read_req;
    io_prep_pread(&read_req, src_fd, buffer, fsize, 0);
    if (io_submit(aio_context, 1, &read_iocbs) != 1)
    {
        printf("Failed to submit src file read request...EXITING\n");
        return -1;
    }

    // Spin until read completes
    struct io_event read_events[1];
    while (io_getevents(aio_context, 0, 1, &read_events[0], NULL) == 0) {} //spin
    if (read_events[0].res < 0)
    {
        printf("Failed to read src file...EXITING\n");
        return -1;
    }
    close(src_fd);

    // Submit write request to dest file
    int dest_fd = open(dest_fp, O_RDWR | O_CREAT | O_DIRECT, "a");
    if (dest_fd < 0)
    {
        printf("Failed to open dest file...EXITING\n");
        return -1;
    }
    struct iocb write_req;
    struct iocb *write_iocbs = &write_req;
    io_prep_pwrite(&write_req, dest_fd, buffer, fsize, 0);
    if (io_submit(aio_context, 1, &write_iocbs) != 1)
    {
        printf("Failed to submit dest file write request...EXITING\n");
        return -1;
    }

    // Spin until write completes, then clean up
    struct io_event write_events[1];
    while (io_getevents(aio_context, 0, 1, &write_events[0], NULL) == 0) {} //spin
    if (write_events[0].res < 0)
    {
        printf("Failed to write dest file...EXITING\n");
        return -1;
    }
    io_destroy(aio_context);
    free(buffer);
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
