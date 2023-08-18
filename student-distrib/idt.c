/* idt.c - IDT setup code
 * vim:ts=4 noexpandtab
 */

#include "idt.h"
#include "x86_desc.h"
/* void idt_init()
 * initializes the IDT
 * Inputs: None
 * Outputs: None
 * Side Effects: Initializes the IDT
 */
 void idt_init(){
    idt_desc_t      generic_fault_desc;     // create a stand-in IDT descriptor
    idt_desc_t      kb_handler_desc;        // create kb handler descriptor
    idt_desc_t      mouse_handler_desc;        // create mouse handler descriptor
    idt_desc_t      rtc_handler_desc;       // create rtc handler descriptor
    idt_desc_t      pit_handler_desc;       // create pit handler descriptor


    SET_IDT_ENTRY(generic_fault_desc, &generic_fault_linkage);// set the handler

    SET_IDT_ENTRY(kb_handler_desc, &kb_handler_linkage);      // set the handler
    kb_handler_desc.seg_selector = ((uint16_t) KERNEL_CS);    // set segment to KERNEL CS
    kb_handler_desc.reserved4 = 0;
    kb_handler_desc.reserved3 = 0;
    kb_handler_desc.reserved2 = 1;    // for interrupt gates this is just how it is..
    kb_handler_desc.reserved1 = 1;
    kb_handler_desc.size      = 1;    // seems that gate size=1 -> 32-bit and size=0 -> 16-bit
    kb_handler_desc.reserved0 = 0;
    kb_handler_desc.dpl       = 0;    // DPL is 0 since this is an interrupt gate
    kb_handler_desc.present   = 1;    // mark IDT entry as PRESENT!

    SET_IDT_ENTRY(mouse_handler_desc, &mouse_handler_linkage);   // set the handler
    mouse_handler_desc.seg_selector = ((uint16_t) KERNEL_CS);    // set segment to KERNEL CS
    mouse_handler_desc.reserved4 = 0;
    mouse_handler_desc.reserved3 = 0;
    mouse_handler_desc.reserved2 = 1;   // for interrupt gates this is just how it is..
    mouse_handler_desc.reserved1 = 1;
    mouse_handler_desc.size     = 1;    // seems that gate size=1 -> 32-bit and size=0 -> 16-bit
    mouse_handler_desc.reserved0 = 0;
    mouse_handler_desc.dpl      = 0;    // DPL is 0 since this is kernel code?
    mouse_handler_desc.present  = 1;    // mark IDT entry as PRESENT!

    SET_IDT_ENTRY(rtc_handler_desc, &rtc_handler_linkage);     // set the rtc handler
    rtc_handler_desc.seg_selector = ((uint16_t) KERNEL_CS);    // set segment to KERNEL CS
    rtc_handler_desc.reserved4 = 0;
    rtc_handler_desc.reserved3 = 0;
    rtc_handler_desc.reserved2 = 1;    // for interrupt gates this is just how it is..
    rtc_handler_desc.reserved1 = 1;
    rtc_handler_desc.size      = 1;    // seems that gate size=1 -> 32-bit and size=0 -> 16-bit
    rtc_handler_desc.reserved0 = 0;
    rtc_handler_desc.dpl       = 0;    // DPL is 0 since this is an interrupt gate
    rtc_handler_desc.present   = 1;    // mark IDT entry as PRESENT!

    SET_IDT_ENTRY(pit_handler_desc, &pit_handler_linkage);    // set the pit handler
    pit_handler_desc.seg_selector = ((uint16_t) KERNEL_CS);    // set segment to KERNEL CS
    pit_handler_desc.reserved4 = 0;
    pit_handler_desc.reserved3 = 0;
    pit_handler_desc.reserved2 = 1;    // for interrupt gates this is just how it is..
    pit_handler_desc.reserved1 = 1;
    pit_handler_desc.size      = 1;    // seems that gate size=1 -> 32-bit and size=0 -> 16-bit
    pit_handler_desc.reserved0 = 0;
    pit_handler_desc.dpl       = 0;    // DPL is 0 since this is an interrupt gate
    pit_handler_desc.present   = 1;    // mark IDT entry as PRESENT!

    // We load the first 20 entries of the IDT since they're standardized by intel
    // except 15, we don't want to make a handler for that
    int i;
    for(i = 0; i < 20; i++){
        idt[i] = generic_fault_desc;
        idt[i].seg_selector = ((uint16_t) KERNEL_CS);    // set segment to KERNEL CS
        idt[i].reserved4 = 0;
        idt[i].reserved3 = 0;
        idt[i].reserved2 = 1;    // for interrupt gates this is just how it is..
        idt[i].reserved1 = 1;
        idt[i].size      = 1;    // seems that gate size=1 -> 32-bit and size=0 -> 16-bit
        idt[i].reserved0 = 0;
        idt[i].dpl       = 0;    // DPL is 0 since user level should not be able to run this
        idt[i].present   = 1;    // mark IDT entry as PRESENT!
    }
    idt[15].present = 0;         // Unmark idt15, since this is intel reserved.


    SET_IDT_ENTRY(idt[0], &divide_err_linkage); // divide error
    SET_IDT_ENTRY(idt[1], &reserved_err);       // Intel reserved
    SET_IDT_ENTRY(idt[2], &NMI_int);            // NMI - this will change later!
    SET_IDT_ENTRY(idt[3], &divide_err);         // Breakpoint - should be a TRAP
    SET_IDT_ENTRY(idt[4], &overflow_int);;      // Overflow - should be a TRAP
    SET_IDT_ENTRY(idt[5], &BR_exceeded);        // BOUND range exceeded
    SET_IDT_ENTRY(idt[6], &invalid_opcode);     // Invalid Opcode
    SET_IDT_ENTRY(idt[8], &double_fault_linkage);   // double fault
    SET_IDT_ENTRY(idt[10], &invalid_tss);       // invalid_tss
    SET_IDT_ENTRY(idt[11], &segment_not_present);
    SET_IDT_ENTRY(idt[12], &stack_segment_fault);
    SET_IDT_ENTRY(idt[13], &general_protection);
    SET_IDT_ENTRY(idt[14], &page_fault);        // page-fault - must be handled differently later!
    /* no entry for 15, since this is intel-reserved. */ 
    SET_IDT_ENTRY(idt[16], &fpu_fault);         // x87 FPU faults
    SET_IDT_ENTRY(idt[17], &alignment_check);   // alignment check fault - error code
    SET_IDT_ENTRY(idt[18], &machine_check);     // machine check - should be ABORT?
    SET_IDT_ENTRY(idt[19], &SSE_fault);         // SSE instruction fault
    // idt[19] = generic_fault_desc;  // SSE instruction fault
    
    /* we do not use IDT20-31, since these are intel reserved. */

    idt[0x20] = pit_handler_desc;   // PIT      -> IRQ0 -> port 0x20 
    idt[0x21] = kb_handler_desc;    // Keyboard -> IRQ1 -> port 0x21
    idt[0x28] = rtc_handler_desc;   // RTC      -> IRQ8 -> port 0x28
    idt[0x2C] = mouse_handler_desc;  // 

    /* initialize System Call IDT entry. sys calls are 0x80 in idt */
    idt[0x80].seg_selector = ((uint16_t) KERNEL_CS);    // set segment to KERNEL CS
    idt[0x80].reserved4 = 0;
    idt[0x80].reserved3 = 1;
    idt[0x80].reserved2 = 1;    // for interrupt gates this is just how it is..
    idt[0x80].reserved1 = 1;
    idt[0x80].size      = 1;    // seems that gate size=1 -> 32-bit and size=0 -> 16-bit
    idt[0x80].reserved0 = 0;
    idt[0x80].dpl       = 3;    // DPL is 3 bc user level code can access System Calls
    idt[0x80].present   = 1;    // DPL is 0 since this is kernel code?
    SET_IDT_ENTRY(idt[0x80], &syscall_handler); // load sys_call_linkage to IDT

    lidt(idt_desc_ptr);         // load the IDT we've set up
}
