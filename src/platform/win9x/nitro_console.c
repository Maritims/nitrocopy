#include "nitro_console.h"

#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <windows.h>

int nitro_console_get_width(void) {
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
}

void nitro_console_move_cursor_up(int rows) {
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
}

void nitro_console_clear_from_cursor_down(void) {
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
}
