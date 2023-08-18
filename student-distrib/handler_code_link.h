/* handler_link.h - interfaces for interrupt handlers
 * vim:ts=4 noexpandtab
 */

#include "types.h"
#ifndef ASM

// Declare handlers and their linkages
extern void generic_fault_code();    
extern void generic_fault_code_linkage();
extern void double_fault();
extern void double_fault_linkage();
#endif /* ASM */
