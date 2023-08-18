/* idt.h - Interrupt Descriptor Table setup - loads the interrupt handlers
 * vim:ts=4 noexpandtab
 */

#include "x86_desc.h"
#include "lib.h"
#include "inthandlers.h"
#include "i8259.h"
#include "keyboard.h"
#include "mouse.h"
#include "rtc.h"
#include "syscall_handler.h"
#include "handler_link.h"
#include "handler_code_link.h"


// Fills in IDT and loads it using lidt
void idt_init();    
