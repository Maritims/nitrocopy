#include "console.h"

#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>

int console_get_width(void) {
    struct winsize ws;

    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        perror("ioctl");
        return 1;
    }

    return ws.ws_col;
}

void console_move_cursor_up(int rows) {
    int row;
    for(row = 0; row < rows; row++) {
        printf("\033[A");
    }
}

void console_clear_from_cursor_down(void) {
    printf("\033[J");
}

