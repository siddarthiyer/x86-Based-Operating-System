/* terminal.c - terminal driver functions
 * vim:ts=4 noexpandtab
 */

#include "syscall.h"
#include "terminal.h"
#include "paging.h"
#include "lib.h"
#include "bga.h"

int cur_term = -1;

// pointer to the keyboard's buffer
volatile static char* kbdbuf;
// size of the keyboard buf
volatile static  int kbdbufsize;
// pointer to the current index of the keyboard buffer
volatile static int* kbdbufidx;
// pointer to boolean that determines whether terminal_read shoudl read buffer
// acts like a rudimentary spinlock, for now
// static int* bufready;



/** terminal_open
 * DESCRIPTION: Initializes terminal 
 * INPUTS:
 *      volatile char* input_buf: pointer to buffer from which the terminal takes input (typically kernel kb buf)
 *      int bufsize: maximum size of the input buffer
 *      volatile int* cur_idx: pointer to an int storing where next char in buf would go
 * OUTPUTS:
 *      0 if success, -1 if fail
 * SIDE EFFECTS: Initializes static pointers to point at the given values, allows terminal to 
 * read from and interact with/clear the buffer (typically keyboard buffer)
*/
int terminal_open(){
    kbdbuf = get_kb_buf_ptr();
    kbdbufsize = get_kbdbufsize();
    kbdbufidx = get_kb_buf_idx_ptr();
    if((kbdbuf == 0) || (kbdbufidx < 0) || (kbdbufsize == 0)){
        printf("terminal_open error: invalid input pointers or values\n");
        return -1;
    }
    show_cursor();
    return 0;
}

/** terminal_close
 * DESCRIPTION: Shuts down terminal and clears static values
 * INPUTS:
 *      NONE
 * OUTPUTS:
 *      0 if success, -1 if fail (always success)
 * SIDE EFFECTS: Clears static pointers and zeroes out the keyboard buffer.
*/
int terminal_close(int32_t fd){
    if (fd >= MAX_FD || fd <= 1) return -1;
    int i;
    for(i = 0; i < kbdbufsize; i++){    // kernel buffer-clearing
        kbdbuf[i] = 0x00;
    }
    kbdbuf = 0;
    kbdbufsize = 0;
    kbdbufidx = 0;
    return 0;
}

/** terminal_read
 * DESCRIPTION: Mirrors input buffer into output buffer until first '\n' character. IS BLOCKING!
 * INPUTS:
 *      int fd: unused input for file descriptor
 *      char* buf: pointer to output buffer to which the contents of the input buffer are copied
 *      int n: size of the output buffer
 * OUTPUTS:
 *      Number of bytes copied if success, -1 if fail
 * SIDE EFFECTS: Continually reads the newest addition to the input buffer. If '\n', copy to output buffer and return.
 * Zeroes out the input buffer first. Copies up to n or limit of the input buffer, whichever is less.
*/
int terminal_read(int32_t fd, int8_t* buf, int32_t nbytes){
    int copy_bytes;
    int last_idx;
    if((buf == 0)|(nbytes <= 0))
    {
        printf("terminal_read error: invalid buffer input or read size\n");
        return -1;
    }

    update_cursor();
    memset((void*)kbdbuf, 0x00, kbdbufsize);
 
    (*kbdbufidx) =0;  // start from beginning since we just cleared
    last_idx = *kbdbufidx;

    while(terminalState[cur_term].enter_pressed == 0){}   // while the user hasn't hit enter...

    kb_putc('\n');
    kbdbuf[((*kbdbufidx))++] = '\n';
    terminalState[cur_term].enter_pressed = 0;
    copy_bytes = *kbdbufidx;
    if(nbytes < copy_bytes) // number of bytes to copy = min of n-1 and bytes written to kbbuf
    {
        copy_bytes = nbytes;
    }

    // if we exited then it's time to copy from kbdbuf to the external buf
    memcpy(buf, (void*)kbdbuf, copy_bytes);
    memset((void*)kbdbuf, 0x00, kbdbufsize);
    (*kbdbufidx) =0;

    return (copy_bytes);  // return number of chars copied to buf
}

/** terminal_write
 * DESCRIPTION: Outputs to terminal. Effectively, glorified printf.
 * INPUTS:
 *      int fd: unused input for file descriptor
 *      char* buf: pointer to input buffer from which we print.
 *      int n: Number of chars to print from the buffer.
 * OUTPUTS:
 *      Number of bytes printed if success, -1 if fail
 * SIDE EFFECTS: prints up to n values from input buffer to screen. Skips null chars. 
*/
int terminal_write(int32_t fd, const int8_t* buf, int32_t nbytes){
    int retval;
    if((buf == 0) | (nbytes < 0)){
        printf("terminal_write error: invalid buffer input or write size\n");
        return -1;
    }
    while(nbytes-- > 0) // loop n times, counting down
    { 
        retval++;
        if(*buf != '\0'){    // choose to not print null chars
            putc(*(buf)); // print successive characters in buffer
        }
        buf++;
    }
    if (cur_term == vis_term)
    {
        update_cursor();
    }
    return retval;
}

/* void terminal_init();
 * Inputs: none
 * Return Value: none
 * Function: Initializes TerminalState array with default values
 */
void terminal_init() {
    int i;
    vis_term = 0;
    for (i = 0; i < MAX_TERMINALS; i++) {
        memset(terminalState[i].keyboard_buf, '\0', TERM_BUF_SIZE);
        terminalState[i].buf_idx  = 0;
        terminalState[i].screen_x = 0;
        terminalState[i].screen_y = 0;
        terminalState[i].cur_pid  = i;
        terminalState[i].enter_pressed = 0;
    }
}

/* void switch_terminal(uint8_t new_term);
 * Inputs: uint8_t new_term - Terminal to switch to
 * Return Value: none
 * Function: Switches to the requested terminal. Restores previous state from TerminalState array.
 */
void switch_terminal(int new_term) 
{
    if (new_term == vis_term) return;
    
    // save terminal keyboard buffer and idx
    strncpy(terminalState[vis_term].keyboard_buf, (const int8_t *)kbdbuf, TERM_BUF_SIZE);
    terminalState[vis_term].buf_idx = *kbdbufidx;

    // save terminal cursor pos
    terminalState[vis_term].screen_x = screen_x;
    terminalState[vis_term].screen_y = screen_y;


    // save old terminal's screen to video page assigned for it
    // restore new terminal's screen to video page
    // switch execution to new terminal's user program

    memcpy((void *)(0x8C00000+vis_term*0x400000+17920), (void *)(0x8800000+17920), 256000);
    memcpy((void *)(0x8800000+17920), (void *)(0x8C00000+new_term*0x400000+17920), 256000);
    refresh();
    strncpy((int8_t *)kbdbuf, terminalState[new_term].keyboard_buf, TERM_BUF_SIZE);
    *kbdbufidx = terminalState[new_term].buf_idx;

    // save terminal cursor pos
    screen_x = terminalState[new_term].screen_x;
    screen_y = terminalState[new_term].screen_y;
    update_cursor();

    vis_term = new_term;
}
