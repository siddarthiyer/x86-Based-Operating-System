# handler_link.S - set up assembly linkage for interrupt handlers
# vim:ts=4 noexpandtab

#define ASM     1
#include "handler_link.h"

/* assembly linkage for interrupt handler calls
    Allows a standard C function to use iret */
#define INTR_LINK(name,func)     \
    .globl name                 ;\
    name:                       ;\
        pushal                  ;\
        pushfl                  ;\
        call func               ;\
        popfl                   ;\
        popal                   ;\
        iret                    

INTR_LINK(generic_fault_linkage, generic_fault);
INTR_LINK(kb_handler_linkage, kb_handler);
INTR_LINK(divide_err_linkage, divide_err);
INTR_LINK(do_nothing_linkage, do_nothing);
INTR_LINK(rtc_handler_linkage, rtc_handler);
INTR_LINK(mouse_handler_linkage, mouse_handler);
INTR_LINK(pit_handler_linkage, pit_handler);
