/* inthandlers.c - Interrupt handlers and IDT setup
 * vim:ts=4 noexpandtab
 */

#define HALT_EXC    100

#include "inthandlers.h"

/* Interrupt Handlers (general)
 * Prints out the name of the exception or interrupt
 * Then Spins infinitely (most of the time)
 * Inputs: None
 * Outputs: None
 * Side Effects: Prints, then spins infinitely
 */

// handler for faults without codes
void generic_fault(){
    printf("Generic fault occurred.\n");
    /* Spin (nicely, so we don't chew up cycles) */
    asm volatile (".2: hlt; jmp .2;");
}

// unique handler for divide err 0x00
void divide_err(){
    printf("0x00 Divide error\n");
    halt(HALT_EXC);
}

// handler for reserved err 0x01
void reserved_err(){
    printf("0x01 reserved error\n");
    halt(HALT_EXC);
}

// handler for NMI interrupt 0x02
void NMI_int(){
    printf("0x02 NMI_interrupt occurred\n");
    halt(HALT_EXC);
}

// handler for breakpoint 0x03
void breakpoint_int(){
    printf("0x03 breakpoint occurred\n");
    halt(HALT_EXC);
}

// handler for overflow 0x04
void overflow_int(){
    printf("0x04 overflow occurred\n");
    halt(HALT_EXC);
}

// handler for bound range exceeded 0x05
void BR_exceeded(){
    printf("0x05 Bound Range Exceeded\n");
    halt(HALT_EXC);
}

// handler for invalid opcode 0x06
void invalid_opcode(){
    printf("0x06 invalid opcode occurred\n");
    halt(HALT_EXC);
}

// page fault handler 0x08
void double_fault(){
    printf("0x08 double-fault\n");
}

// handler for overflow 0x0A
void invalid_tss(){
    printf("0x0A (10) invalid_tss\n");
    halt(HALT_EXC);
}

// handler for segment not present 0x0B
void segment_not_present(){
    printf("0x0B (11) segment_not_present\n");
    halt(HALT_EXC);
}

// handler for stack-segment fault 0x0C
void stack_segment_fault(){
    printf("0x0C (12) stack-segment fault\n");
    halt(HALT_EXC);
}

// handler for general-protection fault 0x0D
void general_protection(){
    printf("0x0D (13) general-protection fault\n");
    halt(HALT_EXC);
}

// page fault handler 0x0E
void page_fault(){
    printf("0x0E (14) page fault\n");
    halt(HALT_EXC);
}

// handler for FPU fault 0x10
void fpu_fault(){
    printf("0x10 (16) FPU fault\n");
    halt(HALT_EXC);
}

// handler for alignment check fault 0x11
void alignment_check(){
    printf("0x11 (17) alignment_check fault\n");
    halt(HALT_EXC);
}

// handler for machine check fault 0x12
void machine_check(){
    printf("0x12 (18) machine_check fault\n");
    halt(HALT_EXC);
}

// handler for SSE fault 0x13
void SSE_fault(){
    printf("0x13 (19) SSE fault\n");
    halt(HALT_EXC);
}

// handler that does LEGITIMATELY NOTHING
// DOES NOT SPIN
void do_nothing(){
    return;
}

// handler for faults with a code
// pops the code, and displays it.
// BUG: DOES NOT DISPLAY THE RIGHT VALUE!
void generic_fault_code(int arg){
    printf("Generic fault occurred with code %d\n", arg);
    halt(HALT_EXC);
}

// handler for system calls - TEMPORARY
// void sys_call(){    // just for testing!!! may be wrong!!!
//     int arg = 15;
//     asm volatile ("movl %%eax, %0"
//             : "=r" (arg));
//     printf("sys_call soft interrupt called with arg %x\n", arg);
//     //while(1);
// }
// handler for printing if syscall invalid
void invalid_syscall(){    // just for testing!!! may be wrong!!!
    int arg = 15;
    asm volatile ("movl %%eax, %0"
            : "=r" (arg));
    printf("syscall called with invalid parameter 0x%x\n", arg);
}

