/* keyboard.c - keyboard IRQ handler + initkbdbuf
 * vim:ts=4 noexpandtab
 */

#include "terminal.h"
#include "keyboard.h"
#include "bga.h"
#include "speaker.h"
#include "scheduler.h"
#include "pit.h"

extern terminal_t terminalState[3];

// ASCII code of the highest control char (unprintable characters)
#define MAX_CTRL_CHAR_ASCII 0x1F
// ASCII code of the lowest non-text char (unprintable or invalid)
#define MIN_NON_TEXT 0x7F
// Printable ASCII chars are within these two values!

/* Terminal constants */
/* size of the terminal buffer - 128 as per specification */
#define TERM_BUF_SIZE 128
// static buffer for terminal
volatile static char kb_buf[TERM_BUF_SIZE];
volatile static int kb_buf_idx;

// boolean to keep track of the current shift state
static unsigned int shift;
// define character for Lshift down - 0x0F, "shift in"
#define LSH_DN 0x0F
// release char will be 0x80 + press keycode
#define LSH_UP 0x8F
// define character for Rshift down - 0x0E, "shift out"
#define RSH_DN 0x0E
// release char will be 0x80 + press keycode
#define RSH_UP 0x8E
// boolean to keep track of capslock state
static unsigned int capslock;
// define character for Capslock down - 0x20, "Data Link Escape"
// don't need an up character
#define CL_DN 0x10  
// boolean, 1 if capitalize characters, 0 if not
static unsigned int capitalize;

// boolean to keep track of Lctrl state
static unsigned int Lctrl;
// define character for LCTRL down - 0x11, "device control 1"
#define LCTRL_DN 0x11
// release char will be 0x80 + press keycode
#define LCTRL_UP 0x91
// boolean to keep track of Rctrl state
static unsigned int Rctrl;
// define character for RCTRL down - 0x12, "device control 2"
#define RCTRL_DN 0x12
// release char will be 0x80 + press keycode
#define RCTRL_UP 0x92
// overall ctrl state boolean
static unsigned int ctrl;

// boolean for left-alt state
static unsigned int Lalt;
// define character for Lalt down - 0x13, "device control 3"
#define LALT_DN 0x13
// release char will be 0x80 + press keycode
#define LALT_UP 0x93
static unsigned int Ralt;
// define character for Ralt down - 0x14, "device control 4"
#define RALT_DN 0x14
// release char will be 0x80 + press keycode
#define RALT_UP 0x94


/* piano keys */
#define W_DEP  0x9D
#define E_DEP  0xB8
#define I_DEP  0x97
#define O_DEP  0x98
#define A_DEP  0x9E
#define S_DEP  0x9F
#define D_DEP  0xA0
#define J_DEP  0xA4
#define K_DEP  0xA5
#define L_DEP  0xA6
#define P_DEP  0x99
#define SEMICOLON_DEP 0xA7

#define U_ARR  0x95
#define D_ARR  0x96


// overall alt state boolean 
static unsigned int alt;


// array for translating from keycode to printable character
// for control keys, need to keep track of releases
// release char will be 0x80 + press keycode
static unsigned char PS2Set1[256] = // 256 to include every possible keycode
    {0x00,  // code 0 unassigned
    0x1B,   // ESC pressed - ESC control char
    '1',   // code 0x02, #1 - 9
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',   // code 0x0A: #9
    '0',   // code 0x0B: #0
    '-',   // '-'
    '=',    // '='
    0x08,   // backspace
    0x09,   // tab
    'q',    // Q
    'w',    // W
    'e',    // E
    'r',    // R
    't',    // T
    'y',    // Y
    'u',
    'i',  
    'o',
    'p',
    '[',
    ']',
    '\n',   // Enter - 0x0D per ascii?
    LCTRL_DN,   // Lctrl - "device control 1"
    'a',    // A
    's',    // S
    'd',    // D
    'f',
    'g',
    'h',
    'j',
    'k',
    'l',
    ';',
    0x27,   // "'"
    '`',
    LSH_DN,   // Lshift - "shift in"
    0x5c,   // '\'
    'z',
    'x',
    'c',
    'v',
    'b',
    'n',
    'm',
    ',',
    '.',
    '/',
    RSH_DN,   // Rshift - "shift out"
    '*',    // keypad
    LALT_DN,   // Lalt - "device control 3"
    ' ',    // space
    CL_DN,   // capslock - "DLE"
    0x8D,   // F1
    0x81,   // F2
    0x90,   // F3
    0x00,   // F4
    0x00,   // F5
    0x00,   // F6
    0x00,   // F7
    0x00,   // F8
    0x00,   // F9
    0x00,   // F10
    0x00,   // NumLock
    0x00,   // ScrLock
    0x00,    // keypad 7
    U_ARR,  // Up arrow
    0x00,   // keypad 9
    0x00,   // keypad '-'
    0x00,   // keypad 4
    0x00,   // keypad 5
    0x00,   // keypad 6
    0x00,   // keypad +
    0x00,   // keypad 1
    D_ARR,  // down arrow
    0x00,   // keypad 3
    0x00,   // keypad 0
    0x00,   // keypad .
    0x00,   // N/A
    0x00,   // N/A
    0x00,   // N/A
    0x00,   // F11
    0x00,   // F12
    0x00,   // N/A
    0x00,   // N/A
    0x00,   // N/A
    0x00,   // N/A
    0x00,   // whole bunch of N/A (unused)
    0x00,   
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,   // ESC released...  don't care
    0x00,   // 1 released... don't care
    0x00,   // 2 released
    0x00,   
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,   
    W_DEP,    // W released
    E_DEP,    // E released
    0x00,    
    0x00,    // T released
    0x00,   
    0x00,
    I_DEP,    // I released
    O_DEP,    // O released
    P_DEP,
    0x00,
    0x00,
    0x00,
    LCTRL_UP,   // LCTRL released - "device control 2" 0x12 + 0x80
    A_DEP,    // A released
    S_DEP,    // S released
    D_DEP,    // D released
    0x00,    // F released
    0x00, 
    0x00,
    J_DEP,    // J released
    K_DEP,    // K released
    L_DEP,    // L released
    SEMICOLON_DEP,    
    0x00,   // "'"
    0x00,
    LSH_UP,   // Lshift released - 0x0F shift-in + 0x80
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    RSH_UP,     // Rshift released - 0x0E shift-out + 0x80
    0x00,
    LALT_UP,    // Lalt released
    0x00,
    0x00,
    0x00,       // capslock released- but don't care.
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,};

/** kb_init
 * DESCRIPTION: Initializes KB handler
 * INPUTS:
 *      volatile char* dest_buf: pointer to buffer that the keyboard will write inputs to
 *      int bufsize: maximum size of the buffer
 *      volatile int* cur_idx: pointer to an int storing where next char in buf would go. modified by kb_handler.
 * OUTPUTS:
 *      0 if success, -1 if fail
 * SIDE EFFECTS: Initializes static pointers to point at the given values, allows keyboard to 
 * read write to buffers so that other programs/functions (e.g. terminal) can read inputs.
 * Clears input buffer and enables KB interrupts. NEEDS 8259 INIT FIRST!
*/
void kb_init(){
    memset((void*)kb_buf, '\0', TERM_BUF_SIZE);
    // for(i = 0; i < TERM_BUF_SIZE; i++){    // kernel buffer-clearing
    //     kb_buf[i] = 0x00;  
    // }
    kb_buf_idx = 0;
    enable_irq(KB_IRQ);
}

volatile char* get_kb_buf_ptr(){
    return kb_buf;
}

volatile int* get_kb_buf_idx_ptr(){
    return &kb_buf_idx;
}

int get_kbdbufsize(){
    return TERM_BUF_SIZE;
}

/** kb_handler
 * DESCRIPTION: Handles KB interrupts, according to PC/XT Scan Code Set 1
 * INPUTS:
 *      NONE
 * OUTPUTS:
 *      NONE
 * SIDE EFFECTS: Reads scancode from keyboard and prints char to screen
 * - Also copies printed chars to kernel's kb buffer, until buffer is full. 
 * - Handles special keys and cases: ctrl-l blanks screen and clears buffer
 * - Handles shift and capslock and combinations, and capitalization of numbers
*/
void kb_handler(){
    int space_idx;
    int search_length;
    int takes_args;
    int num_hits;
    int hit_idx;
    // printf("kb_handler run ");
    cli();
    send_eoi(KB_IRQ);
    volatile unsigned char keycode;
    keycode = inb(PORT_8042_DATA);
    unsigned char keychar = PS2Set1[(int)keycode];
    // printf("Keyboard did a thing! status: [%x]\n", keycode);


    // if youre reading this, pretend you dont see this goto statement...
    if(pianomode)
    {
        goto PIANO;
    }


    switch(keychar)
    {
        case 0x00   :   // null or we don't care - exit early and do nothing
            sti();  // we're done here, get out
            return;
        case 0x1B   :   // 0x1B corresponds to ESC
            // don't do anything for escape... YET
            sti();  // we're done here, get out
            return;
        case LCTRL_DN   :   // 0x11 is "device control 1" used as LCTRL
            Lctrl = 1;      // Lctrl active
            ctrl = 1;   // update overall ctrl state
            sti();  // we're done here, get out
            return;
        case LCTRL_UP   :   // 0x91 is 0x80 + device control 1, indicating release
            Lctrl = 0;
            ctrl = Lctrl | Rctrl;   // update overall ctrl state
            sti();  // we're done here, get out
            return;
        case RCTRL_DN   :
            Rctrl = 1;
            ctrl = 1;
            sti();  // we're done here, get out
            return;
        case RCTRL_UP   :
            Rctrl = 0;
            ctrl = Lctrl | Rctrl;
            sti();  // we're done here, get out
            return;
        case LALT_DN    :
            Lalt = 1;
            alt = 1;
            sti();  // we're done here, get out
            return;
        case LALT_UP    :
            Lalt = 0;
            alt = Lalt | Ralt;
            sti();  // we're done here, get out
            return;
        case LSH_DN     :   // handle both shifts identically
        case RSH_DN     :
            shift = 1;
            capitalize = 1 - capslock;  // set capitalization to not-capslock
            // printf("LSH_DN, cap is: %d", capitalize);
            sti();  // we're done here, get out
            return;
        case LSH_UP     :   // The two shifts behave in unison, very confusing
        case RSH_UP     :   // when holding both, no release is sent until 2nd is released...
            shift = 0;
            capitalize = capslock;
            // printf("LSH_UP, cap is: %d", capitalize);
            sti();  // we're done here, get out
            return;
        case CL_DN      :
            capslock ^= 1;   // toggle capslock
            capitalize = shift^capslock;  // xor selective inversion
            // printf("capslock is: %d", capslock);
            sti();  // we're done here, get out
            return;

        case 0x08   :   // backspace
            if (lockscreenflag)
            {
                updated = 1;
                if (mode == 0)
                {
                    if (useridx>0)
                    {
                        (useridx)--; // go back one char
                        ubuf[useridx] = 0x00;  // erase it
                    }
                }
                else
                {
                    if (passidx>0)
                    {
                        (passidx)--; // go back one char
                        pbuf[passidx] = 0x00;  // erase it
                    }
                }

            }

            if(kb_buf_idx > 0){ // if we have inputs to backtrack and remove
                (kb_buf_idx)--; // go back one char
                kb_buf[kb_buf_idx] = 0x00;  // erase it
                kb_putc_bksp(); // visually erase it too
                update_cursor();
            }
            // kb_putc(0x08);
            sti();  // we're done here, get out
            return;
        case '\n'   :   // enter - if anything's waiting for keyboard, it's time!
            // but terminal is responsible for figuring out what's in the buffer and clearing it!
            if (lockscreenflag)
            {
                LS_enter = 1;
            }
            if((kb_buf_idx) < (TERM_BUF_SIZE) && term_flag){     // if writing a char won't OVERfill the buffer
                terminalState[vis_term].enter_pressed = 1;
                // moved putc to terminal write, not sure if itll cause dependency issues
                // kb_putc(keychar);  // print the new char
                
                // kb_buf[(kb_buf_idx)++] = *(char*)(&keychar);   // write as char and increment idx
            }
            //*bufready = 1;  // signal to any waiting terminal drivers that the kb_buf is ready
            sti();  // we're done here, get out
            return;
        case 0x09   :   // Tab, hardcoded autocomplete
            if (lockscreenflag)
            {
                if (mode)
                {
                    mode = 0;
                    printLockCursor(0);
                }
                else
                {   
                    mode = 1;
                    printLockCursor(1);
                }
                sti();
                return;
            }
            if(kb_buf_idx == 0){    // If there's nothin in the buffer
                sti();      // nothing to autocomplete so just sti->return
                return; 
            }
            for(space_idx = 0; space_idx < kb_buf_idx; space_idx ++){
                if(kb_buf[space_idx] == ' '){  // If we found a space
                    if (space_idx == kb_buf_idx -1)   // If we're at the end
                    {   // do nothing if the space is at the end.
                        sti();
                        return;
                    }
                    space_idx ++;   // iterate to the next character 
                    break;  // space_idx should now contain index of the space.
                }
                else if (space_idx == kb_buf_idx -1)   // If we're at the end, and didn't find space
                {
                    space_idx = 0; // give up and conclude there are no spaces
                    break;
                }
            }
            if(space_idx > 0){  // if we're autocompleting an arg, only do so if command takes args.
                takes_args = 0; // Default - set takes_args to 0
                if(strncmp("cat", (int8_t*)(kb_buf), 3) == 0){  // if buffer CONTAINS cat from beginning
                    takes_args = 1;
                }
                else if(strncmp("grep", (int8_t*)(kb_buf), 4) == 0){
                    takes_args = 1;
                }
                else if(strncmp("sigtest", (int8_t*)(kb_buf), 7) == 0){
                    takes_args = 1;
                }
                else{   // Nothing to be done
                    sti();
                    return;
                }
            }
            else {  // If space_idx = 0, then we always autocomplete.
                takes_args = 1; 
            }
            if(takes_args) {
                num_hits = 0;
                hit_idx = -1;   // Indicates no hits, by default
                search_length = kb_buf_idx - space_idx; //  Number of indices to check - everything after first space
                if(strncmp("cat", (int8_t*)(kb_buf + space_idx), search_length) == 0){   // check if we're a subset of "cat"
                    num_hits ++;
                    hit_idx = 0;
                }
                if(strncmp("counter", (int8_t*)(kb_buf + space_idx), search_length) == 0){  // check if we're a subset of "counter"
                    if(num_hits > 0){
                        kb_printf("\ncat counter \n391OS> %s", kb_buf);
                        update_cursor();
                        sti();
                        return;
                    }
                    else {
                        num_hits++;
                        hit_idx = 1;                    
                    }
                }
                if(strncmp("fish", (int8_t*)(kb_buf + space_idx), search_length) == 0){  // check if we're a subset of "fish"
                    num_hits ++;
                    hit_idx = 2;
                }
                if(strncmp("frame0.txt", (int8_t*)(kb_buf + space_idx), search_length) == 0){  // check if we're a subset of "frame0.txt"
                    if(space_idx > 0) { // If there's been a space (thus, this would be a valid arg)
                        if(num_hits > 0){
                            kb_printf("\nfish frame0.txt frame1.txt \n391OS> %s", kb_buf);
                            update_cursor();
                            sti();
                            return;
                        }
                        else {
                            num_hits++;
                            hit_idx = 3;   
                        }
                    }
                }
                if(strncmp("frame1.txt", (int8_t*)(kb_buf + space_idx), search_length) == 0){  // check if we're a subset of "frame1.txt"
                    if(space_idx > 0) { // If there's been a space (thus, this would be a valid arg)
                        if(num_hits > 0){   // If fish was an option, we'dve already printed it
                            kb_printf("\nframe0.txt frame1.txt \n391OS> %s", kb_buf);
                            update_cursor();
                            sti();
                            return;
                        }
                        else {
                            num_hits++;
                            hit_idx = 4;   
                        }
                    }
                }
                if(strncmp("shell", (int8_t*)(kb_buf + space_idx), search_length) == 0){  // check if we're a subset of "shell"
                    num_hits ++;
                    hit_idx = 5;
                }
                if(strncmp("sigtest", (int8_t*)(kb_buf + space_idx), search_length) == 0){  // check if we're a subset of "sigtest"
                    if(num_hits > 0){   // If fish was an option, we'dve already printed it
                        kb_printf("\nshell sigtest syserr \n391OS> %s", kb_buf);
                        update_cursor();
                        sti();
                        return;
                    }
                    else {
                        num_hits++;
                        hit_idx = 6;   
                    }
                }
                else if(strncmp("syserr", (int8_t*)(kb_buf + space_idx), search_length) == 0){  // check if we're a subset of "syserr"
                    while(kb_buf_idx > space_idx){ // if we have inputs to backtrack and remove
                        kb_buf_idx--;
                        kb_buf[kb_buf_idx] = '\0';  // clear the char from buf
                        kb_putc_bksp(); // visually erase it too
                    }
                    // sync current terminal with the autocompleted value
                    strncpy((int8_t*)(kb_buf + space_idx), "syserr", 6); // put "syserr" into it
                    kb_printf((int8_t*)kb_buf + space_idx);
                    kb_buf_idx += 6; // length of "syserr"
                    update_cursor();
                }
                else if(strncmp("grep", (int8_t*)(kb_buf + space_idx), search_length) == 0){  // check if we're a subset of "grep"
                    // none of the above fancyness since no possibility of multiple hits.
                    while(kb_buf_idx > space_idx){ // if we have inputs to backtrack and remove
                        kb_buf_idx--;
                        kb_buf[kb_buf_idx] = '\0';  // clear the char from buf
                        kb_putc_bksp(); // visually erase it too
                    }
                    // sync current terminal with the autocompleted value
                    strncpy((int8_t*)(kb_buf + space_idx), "grep", 4); // put "grep" into it
                    kb_printf((int8_t*)kb_buf + space_idx);
                    kb_buf_idx += 4; // length of "grep"
                    update_cursor();
                    sti();
                    return;
                }
                else if(strncmp("hello", (int8_t*)(kb_buf + space_idx), search_length) == 0){  // check if we're a subset of "hello"
                    while(kb_buf_idx > space_idx){ // if we have inputs to backtrack and remove
                        kb_buf_idx--;
                        kb_buf[kb_buf_idx] = '\0';  // clear the char from buf
                        kb_putc_bksp(); // visually erase it too
                    }
                    // sync current terminal with the autocompleted value
                    strncpy((int8_t*)(kb_buf + space_idx), "hello", 5); // put "hello" into it
                    kb_printf((int8_t*)kb_buf + space_idx);
                    kb_buf_idx += 5; // length of "hello"
                    update_cursor();
                }
                else if(strncmp("ls", (int8_t*)(kb_buf + space_idx), search_length) == 0){  // check if we're a subset of "ls"
                    while(kb_buf_idx > space_idx){ // if we have inputs to backtrack and remove
                        kb_buf_idx--;
                        kb_buf[kb_buf_idx] = '\0';  // clear the char from buf
                        kb_putc_bksp(); // visually erase it too
                    }
                    // sync current terminal with the autocompleted value
                    strncpy((int8_t*)(kb_buf + space_idx), "ls", 2); // put "ls" into it
                    kb_printf((int8_t*)kb_buf + space_idx);
                    kb_buf_idx += 2; // length of "ls"
                    update_cursor();
                }
                else if(strncmp("pingpong", (int8_t*)(kb_buf + space_idx), search_length) == 0){  // check if we're a subset of "pingpong"
                    while(kb_buf_idx > space_idx){ // if we have inputs to backtrack and remove
                        kb_buf_idx--;
                        kb_buf[kb_buf_idx] = '\0';  // clear the char from buf
                        kb_putc_bksp(); // visually erase it too
                    }
                    // sync current terminal with the autocompleted value
                    strncpy((int8_t*)(kb_buf + space_idx), "pingpong", 8); // put "pingpong" into it
                    kb_printf((int8_t*)kb_buf + space_idx);
                    kb_buf_idx += 8; // length of "pingpong"
                    update_cursor();
                }
                else if(strncmp("testprint", (int8_t*)(kb_buf + space_idx), search_length) == 0){  // check if we're a subset of "testprint"
                    while(kb_buf_idx > space_idx){ // if we have inputs to backtrack and remove
                        kb_buf_idx--;
                        kb_buf[kb_buf_idx] = '\0';  // clear the char from buf
                        kb_putc_bksp(); // visually erase it too
                    }
                    // sync current terminal with the autocompleted value
                    strncpy((int8_t*)(kb_buf + space_idx), "testprint", 9); // put "testprint" into it
                    kb_printf((int8_t*)kb_buf + space_idx);
                    kb_buf_idx += 9; // length of "testprint"
                    update_cursor();
                }
                else if(strncmp("exit", (int8_t*)(kb_buf + space_idx), search_length) == 0){  // check if we're a subset of "exit"
                    if(space_idx == 0){ // Exit cannot be a later arg
                        // exit can only be a first-level arg, never second
                        while(kb_buf_idx > space_idx){ // if we have inputs to backtrack and remove
                            kb_buf_idx--;
                            kb_buf[kb_buf_idx] = '\0';  // clear the char from buf
                            kb_putc_bksp(); // visually erase it too
                        }
                        // sync current terminal with the autocompleted value
                        strncpy((int8_t*)(kb_buf + space_idx), "exit", 4); // put "exit" into it
                        kb_printf((int8_t*)kb_buf + space_idx);
                        kb_buf_idx += 4; // length of "exit"
                        update_cursor();
                        sti();
                        return;
                    }
                }
            }
            if (num_hits > 1){
                hit_idx = -1;   // declare no proper index if multiple hits.
            }
            if(num_hits > 0){
                if(hit_idx != -1){
                    while(kb_buf_idx > space_idx){ // if we have inputs to backtrack and remove
                        kb_buf_idx--;
                        kb_buf[kb_buf_idx] = '\0';  // clear the char from buf
                        kb_putc_bksp(); // visually erase it too
                    }
                    switch (hit_idx)
                    {
                        case 0:
                            // sync current terminal with the autocompleted value
                            strncpy((int8_t*)(kb_buf + space_idx), "cat", 3); // put "cat" into it
                            kb_printf((int8_t*)kb_buf + space_idx);
                            kb_buf_idx += 3; // length of "cat"
                            break;
                        case 1:
                            // sync current terminal with the autocompleted value
                            strncpy((int8_t*)(kb_buf + space_idx), "counter", 7); // put "counter" into it
                            kb_printf((int8_t*)kb_buf + space_idx);
                            kb_buf_idx += 7; // length of "counter"
                            break;
                        case 2:
                            strncpy((int8_t*)(kb_buf + space_idx), "fish", 4); // put "fish" into it
                            kb_printf((int8_t*)kb_buf + space_idx);
                            kb_buf_idx += 4; // length of "fish"
                            break;
                        case 3: 
                            // sync current terminal with the autocompleted value
                            strncpy((int8_t*)(kb_buf + space_idx), "frame0.txt", 10); // put "frame0.txt" into it
                            kb_printf((int8_t*)kb_buf + space_idx);
                            kb_buf_idx += 10; // length of "frame0.txt"
                            break;
                        case 4:
                            // sync current terminal with the autocompleted value
                            strncpy((int8_t*)(kb_buf + space_idx), "frame1.txt", 10); // put "frame1.txt" into it
                            kb_printf((int8_t*)kb_buf + space_idx);
                            kb_buf_idx += 10; // length of "frame0.txt"
                            break;
                        case 5:
                            strncpy((int8_t*)(kb_buf + space_idx), "shell", 5); // put "shell" into it
                            kb_printf((int8_t*)kb_buf + space_idx);
                            kb_buf_idx += 5; // length of "shell"
                            break;
                        case 6:
                            strncpy((int8_t*)(kb_buf + space_idx), "sigtest", 7); // put "sigtest" into it
                            kb_printf((int8_t*)kb_buf + space_idx);
                            kb_buf_idx += 7; // length of "sigtest"                    
                            break;
                        default:
                            break;
                    }
                    update_cursor();
                }
            }
            sti();
            return;
        default :
            break;
    }
    // if we got here, none of the special/modifier keys were the new keypress.
    // special cases for combos?
    if(ctrl){
        if(keychar == 'l'){  //ctrl-l
            // int i;
            clear();    // clear the screen
            kb_putc_reset(); // RESET CURSOR POSITION!
            update_cursor();
            memset((void*)kb_buf, 0x00, TERM_BUF_SIZE);// clear kbd buf
            // for( i = 0; i < TERM_BUF_SIZE; i++){
            //     kb_buf[i] = 0x00;   // clear the kbd buf
            // }
            kb_buf_idx = 0; // reset index;
            sti();  // we're done here, get out
            return;
        }
    }

    if(alt) {
        switch(keychar) {
            case 0x8D: // F1
                switch_terminal(0);
                sti();
                return;

            case 0x81: //F2
                switch_terminal(1);
                sti();
                return;

            case 0x90: //F3
                switch_terminal(2);
                sti();
                return;
        }
    }
    // for all remaining chars that aren't releases or don't cares...
    if(((unsigned)keychar > MAX_CTRL_CHAR_ASCII) && ((unsigned)keychar < MIN_NON_TEXT)){
        // printf("... printable\n");
        if(capitalize){ // if we need to capitalize...
            if( (keychar >= 'a') && (keychar <= 'z')){  // if its lowercase text...
                keychar -= 0x20;    // subtracting 0x20 from the char value changes lower-> upper case ASCII
            }
        }
        if(shift){  // switch these with their symbols when shift held, but not capslock
            switch (keychar)
            {
                case '`':
                    keychar = '~';
                    break;
                case '1':
                    keychar = '!';
                    break;
                case '2':
                    keychar = '@';
                    break;
                case '3':
                    keychar = '#';
                    break;
                case '4':
                    keychar = '$';
                    break;
                case '5':
                    keychar = '%';
                    break;
                case '6':
                    keychar = '^';
                    break;
                case '7':
                    keychar = '&';
                    break;
                case '8':
                    keychar = '*';
                    break;
                case '9':
                    keychar = '(';
                    break;
                case '0':
                    keychar = ')';
                    break;
                case '-':
                    keychar = '_';
                    break;
                case '+':
                    keychar = '=';
                    break;
                case '[':
                    keychar = '{';
                    break;
                case ']':
                    keychar = '}';
                    break;
                case 0x5c:   // '\'
                    keychar = '|';
                    break;
                case ';':
                    keychar = ':';
                    break;
                case 0x27:  // "'"
                    keychar = '"';
                    break;
                case ',':
                    keychar = '<';
                    break;
                case '.':
                    keychar = '>';
                    break;
                case '/':
                    keychar = '?';
                    break;
                default:
                    break;
            }
        }

        if (lockscreenflag)
        {
            updated = 1;
            if (mode == 0)
            {
                if (useridx<15)
                {
                    ubuf[(useridx)++] = *(char*)(&keychar);
                }
            }
            else
            {
                if (passidx<15)
                {
                    pbuf[(passidx)++] = *(char*)(&keychar);
                }
            }

        }

        PIANO:
        if (pianomode)
        {
            sti();

            switch(keychar)
            {
                case 'a':
                    play_sound(note2freq('C', cur_octave));
                    break;

                case 'w':
                    play_sound(note2freq('c', cur_octave));
                    break;
                
                case 's':
                    play_sound(note2freq('D', cur_octave));
                    break;

                case 'e':
                    play_sound(note2freq('d', cur_octave));
                    break;

                case 'd':
                    play_sound(note2freq('E', cur_octave));
                    break;

                case 'j':
                    play_sound(note2freq('F', cur_octave));
                    break;

                case 'i':
                    play_sound(note2freq('f', cur_octave));
                    break;
 
                case 'k':
                    play_sound(note2freq('G', cur_octave));
                    break;

                case 'o':
                    play_sound(note2freq('g', cur_octave));
                    break;
               

                case 'l':
                    play_sound(note2freq('A', cur_octave));
                    break;
                
                case 'p':
                    play_sound(note2freq('a', cur_octave));
                    break; 
                
                case ';':
                    play_sound(note2freq('B', cur_octave));
                    break;


                case A_DEP:
                case W_DEP:
                case S_DEP:
                case E_DEP:
                case D_DEP:
                case SEMICOLON_DEP:
                case P_DEP:
                case J_DEP:
                case I_DEP:
                case K_DEP:
                case O_DEP:
                case L_DEP:
                    speaker_mute();
                    break;

                case U_ARR:
                    piano_change_octave(cur_octave + 1);
                    break;
                
                case D_ARR:
                    piano_change_octave(cur_octave - 1);
                    break;
                
            }

        }
        else
        {
    
            if((kb_buf_idx) < (TERM_BUF_SIZE - 1)){     // if writing a char won't fill the buffer
                kb_putc(keychar);  // print the new char
                update_cursor();
                kb_buf[(kb_buf_idx)++] = *(char*)(&keychar);   // write as char and increment idx
            }
        }
    }
    sti();
    
    return;
}
