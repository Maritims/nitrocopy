#ifndef NITRO_CONSOLE_COMPAT_H
#define NITRO_CONSOLE_COMPAT_H

int nitro_console_get_width(void);

void nitro_console_move_cursor_up(int rows);

void nitro_console_clear_from_cursor_down(void);

#endif
