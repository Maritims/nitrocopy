#include <unistd.h>
#include "nitro_console_compat.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/ioctl.h>
#endif

int main(void) {
    const char* msg = "This is a very long line that should wrap multiple times on the console depending on its width.";
    printf("%s", msg);

    /*return vc->line_count == 1 ? 0 : 1;*/
    return 0;
}
