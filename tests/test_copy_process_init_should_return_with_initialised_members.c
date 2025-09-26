#include "copy_process.h"
#include <assert.h>

int main(void) {
    copy_process_t* copy_process = copy_process_init(0);

    assert(copy_process_get_total_size(copy_process) == 0);
    assert(copy_process_get_total_files(copy_process) == 0);
    assert(copy_process_get_bytes_copied(copy_process) == 0);
    assert(copy_process_get_files_processed(copy_process) == 0);
    assert(copy_process_get_overwrite(copy_process) == 0);

    return 0;
}
