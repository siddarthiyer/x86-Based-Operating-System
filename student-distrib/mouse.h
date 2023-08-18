#include "lib.h"
#include "x86_desc.h"
#include "i8259.h"

extern void mouse_handler(void);
void mouse_wait(uint8_t a_type);
void mouse_write(uint8_t a_write);
uint8_t mouse_read();
extern void mouse_init();
int cursorInBounds(int xmin, int xmax, int ymin, int ymax);
