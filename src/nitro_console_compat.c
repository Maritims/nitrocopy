#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include "nitro_console_compat.h"

#if defined(_WIN32)
    #include <windows.h>
#else
    #include <sys/ioctl.h>
#endif

static int _nitro_get_console_width(void) {
#if defined(_WIN32)
    HANDLE                      hConsole;
    CONSOLE_SCREEN_BUFFER_INFO  csbi;
    int                         rows;
    
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if(hConsole == NULL) {
        return 1;
    }

    if(!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return 1;
    }

    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
#else
    struct winsize ws;

    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        perror("ioctl");
        return 1;
    }

    return ws.ws_col;
#endif
}

static void _nitro_move_cursor_up(int rows) {
#if defined(_WIN32)
    HANDLE                      hConsole;
    CONSOLE_SCREEN_BUFFER_INFO  csbi;
    COORD                       cursor;

    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if(hConsole == NULL) {
        return;
    }

    if(!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return;
    }

    cursor = csbi.dwCursorPosition;
    cursor.Y -= rows;
    if(cursor.Y < 0) {
        cursor.Y = 0;
    }

    SetConsoleCursorPosition(hConsole, cursor);
#else
    int row;
    for(row = 0; row < rows; row++) {
        printf("\033[A");
    }
#endif
}

static void _nitro_clear_from_cursor_down(void) {
#if defined(_WIN32)
    HANDLE                      hConsole;
    CONSOLE_SCREEN_BUFFER_INFO  csbi;
    COORD                       cursor;
    DWORD                       cells;
    DWORD                       written;

    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if(hConsole == NULL) {
        return;
    }

    if(!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return;
    }

    cursor = csbi.dwCursorPosition;
    cells = (csbi.dwSize.Y - cursor.Y) * csbi.dwSize.X;

    FillConsoleOutputCharacter(hConsole, ' ', cells, cursor, &written);
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, cells, cursor, &written);
#else
    printf("\033[J");
#endif
}

void nitro_clear_line(int previous_line_length) {
    int line_width;
    int rows;

    line_width = _nitro_get_console_width();
    
    if(previous_line_length > 0) {
        rows = (previous_line_length + line_width - 1) / line_width;
        _nitro_move_cursor_up(rows);
        _nitro_clear_from_cursor_down();
    }
}
