/* handler_link.h - interfaces for interrupt handlers
 * vim:ts=4 noexpandtab
 */

#include "types.h"
#ifndef ASM

// declare handlers and their linkages
extern void generic_fault();    
extern void generic_fault_linkage();
extern void kb_handler();
extern void kb_handler_linkage();
extern void divide_err();
extern void divide_err_linkage();
extern void do_nothing();
extern void do_nothing_linkage();
extern void rtc_handler();
extern void rtc_handler_linkage();
extern void mouse_handler();
extern void mouse_handler_linkage();
extern void pit_handler();
extern void pit_handler_linkage();
// extern void generic_fault_code();
// extern void generic_fault_code_linkage();
#endif /* ASM */
