/* inthandlers.h - Interrupt handlers and IDT setup
 * vim:ts=4 noexpandtab
 */

#include "x86_desc.h"
#include "i8259.h"
#include "keyboard.h"
#include "rtc.h"
#include "handler_link.h"
#include "handler_code_link.h"
#include "lib.h"

extern int32_t halt (uint8_t status);
// Declare all of the interrupt handlers
// names self explanatory
void generic_fault();
void divide_err();
void reserved_err();
void NMI_int();
void breakpoint_int();
void overflow_int();
void BR_exceeded();
void invalid_opcode();
void double_fault();
void invalid_tss();
void segment_not_present();
void stack_segment_fault();
void general_protection();
void page_fault();
void fpu_fault();
void alignment_check();
void machine_check();
void SSE_fault();
void do_nothing();
void generic_fault_code(int arg);
void invalid_syscall();
