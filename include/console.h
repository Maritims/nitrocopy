#ifndef CONSOLE_H
#define CONSOLE_H

int console_get_width(void);

void console_move_cursor_up(int rows);

void console_clear_from_cursor_down(void);

#endif
