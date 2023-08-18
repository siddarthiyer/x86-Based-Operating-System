/* terminal.h - terminal driver definitions 
 * vim:ts=4 noexpandtab
 */

#ifndef _TERMINAL_H
#define _TERMINAL_H



#include "keyboard.h"

#define MAX_TERMINALS   3
#define TERM_BUF_SIZE   128
#define MAX_FD          8

typedef struct __attribute__((packed)) terminal {
    int8_t keyboard_buf[TERM_BUF_SIZE];
    volatile char history[10][TERM_BUF_SIZE];
    int histidx;
    int buf_idx;
    int screen_x;
    int screen_y;
    int cur_pid;
    unsigned int esp0;
    unsigned int ss0;
    volatile int enter_pressed;
} terminal_t;

terminal_t terminalState[3]; // 3 terminals


int vis_term;
extern int cur_term;

extern int terminal_open();
extern int terminal_close(int32_t fd);
extern int terminal_read(int32_t fd, int8_t* buf, int32_t nbytes);
extern int terminal_write(int32_t fd, const int8_t* buf, int32_t nbytes);
void terminal_init();
void switch_terminal(int new_term);

#endif
