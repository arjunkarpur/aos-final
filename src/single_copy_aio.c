#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <libaio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

int do_copy(char *src_fp, char *dest_fp)
{
    // Perform copy of single file using AIO

    // Create io_context
    io_context_t aio_context = 0;
    if (io_setup(100, &aio_context))
    {
        perror("Failed to create aio context...");
        return -1;
    }

    // Get src file size and create buffer
    int src_fd = open(src_fp, O_DIRECT, "rb");
    if (src_fd < 0)
    {
        perror("Failed to open src file...");
        return -1;
    }
    struct stat f_stat;
    if (fstat(src_fd, &f_stat) != 0)
    {
        perror("Failed to get src file size...");
        return -1;
    }
    int fsize = f_stat.st_size;
    char *buffer = mmap(NULL, fsize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (!buffer)
    {
        perror("Failed to mmap buffer...");
        return -1;
    }

    // Submit read request for src file
    struct iocb read_req;
    struct iocb *read_iocbs = &read_req;
    io_prep_pread(&read_req, src_fd, buffer, fsize, 0);
    if (io_submit(aio_context, 1, &read_iocbs) != 1)
    {
        perror("Failed to submit src file read request...");
        return -1;
    }

    // Spin until read completes
    struct io_event read_events[1];
    while (io_getevents(aio_context, 0, 1, &read_events[0], NULL) == 0) {} //spin
    if (read_events[0].res < 0)
    {
        perror("Failed to read src file...");
        return -1;
    }
    close(src_fd);

    // Submit write request to dest file
    int dest_fd = open(dest_fp, O_RDWR | O_CREAT | O_DIRECT, 0666);
    if (dest_fd < 0)
    {
        perror("Failed to open dest file...");
        return -1;
    }
    struct iocb write_req;
    struct iocb *write_iocbs = &write_req;
    io_prep_pwrite(&write_req, dest_fd, buffer, fsize, 0);
    if (io_submit(aio_context, 1, &write_iocbs) != 1)
    {
        perror("Failed to submit dest file write request...");
        return -1;
    }

    // Spin until write completes, then clean up
    struct io_event write_events[1];
    while (io_getevents(aio_context, 0, 1, &write_events[0], NULL) == 0) {} //spin
    if (write_events[0].res < 0)
    {
        perror("Failed to write dest file...");
        return -1;
    }
    if (io_destroy(aio_context) != 0)
    {
        perror("Failed to destroy aio_context");
        return -1;
    }
    return 0;
}


int main(int argc, char *argv[])
{

    // Get command line args
    if (argc < 3) {
        perror("./copy_test src dest\n");
        return -1;
    }

    // Perform copy
    int ret = do_copy(argv[1], argv[2]);
    return ret;
}
