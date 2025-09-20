#include <stdio.h>
#include "nitro_console_compat.h"

#if defined(_WIN32)
    #include <windows.h>
#endif

void nitro_console_clear_line(void) {
#if defined(_WIN32)
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD written;
    DWORD cells;
    COORD cursor;

    if(!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return;
    }

    /* This is where we are right now. */
    cursor = csbi.dwCursorPosition;

    /* This is the number of characters to clear. */
    cells = csbi.dwSize.X - cursor.X;

    /* Clear the characters. */
    FillConsoleOutputCharacter(hConsole, ' ', cells, cursor, &written);

    /* Reset the attributes. */
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, cells, cursor, &written);

    /* Move back to where we were. */
    SetConsoleCursorPosition(hConsole, cursor);
#else
    printf("\033[K");
    fflush(stdout);
#endif
}
