/* keyboard.h - keyboard handler specifically
 * vim:ts=4 noexpandtab
 */

#include "x86_desc.h"
#include "i8259.h"
#include "lib.h"

#define PORT_8042_DATA  0x60
#define PORT_8042_SCREG 0x64
#define KB_IRQ          0x01

extern void kb_init();
extern void kb_handler();

extern volatile char*   get_kb_buf_ptr();
extern volatile int*    get_kb_buf_idx_ptr();
extern int     get_kbdbufsize();
