# handler_link.S - set up assembly linkage for interrupt handlers
# vim:ts=4 noexpandtab

#define ASM     1
#include "handler_code_link.h"

/* assembly linkage for interrupt handler calls
    Specifically for calls with pushed error codes
    Allows a standard C function to use iret */
.globl generic_fault_code_linkage                 ;\
    generic_fault_code_linkage:                   ;\
        pushl   %eax            ;\
        # load the error code to eax 
        movl    4(%esp), %eax   ;\
        pushal                  ;\
        pushfl                  ;\
        # do push error code for handler
        pushl   %eax            ;\
        call    generic_fault_code            ;\
        # do pop error code, eax gets overwritten
        popl    %eax            ;\
        popfl                   ;\
        popal                   ;\
        # restore EAX
        popl    %eax            ;\
        # do pop the error code
        addl    $4, %esp        ;\
        iret                    


.globl double_fault_linkage                 ;\
    double_fault_linkage:                   ;\
        pushl   %eax            ;\
        # load the error code to eax 
        movl    4(%esp), %eax   ;\
        pushal                  ;\
        pushfl                  ;\
        # do push error code for handler
        pushl   %eax            ;\
        call    double_fault            ;\
        # do pop error code, eax gets overwritten
        popl    %eax            ;\
        popfl                   ;\
        popal                   ;\
        # restore EAX
        popl    %eax            ;\
        # do pop the error code
        addl    $4, %esp        ;\
        iret                    
# CODE_INTR_LINK(generic_fault_code_linkage, generic_fault_code);
# CODE_INTR_LINK(generic_fault_linkage, generic_fault);
