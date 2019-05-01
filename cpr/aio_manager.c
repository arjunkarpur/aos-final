#include "aio_manager.h"

int init_aio_manager(aio_manager_t *aio_manager) {
    //#TODO: initialize all fields of aio_manager
    return 0;
}

int start_aio_thread(aio_manager_t *aio_manager) {
    //TODO: create pthread that runs aio_loop
    return 0;
}

int aio_loop(aio_manager_t *aio_manager) {
    //TODO: perform aio reads and writes until finished
    return 0;
}

int add_copy_req(aio_manager_t *aio_manager, copy_request_t *copy_request) {
    //TODO: atomically add a copy request to the aio_manager
    return 0;
}

int wait_for_aio_finish(aio_manager_t *aio_manager) {
    //TODO: atomically set flush bool and wait for pthread to finish
    return 0;
}

