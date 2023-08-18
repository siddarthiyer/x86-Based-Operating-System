/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

#define IRQ_SECONDARY_PIC 0x02  // the secondary PIC lives on IRQ 2 on the primary pic

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* void i8259_init()
 * initializes the PIC
 * Inputs: None
 * Outputs: None
 * Side Effects: Enables the PIC
 */
 void i8259_init(void) {
    // wiki.osdev.org/PIC


    // What does this line do from Lec 10?
    // i8259A_auto_eoi = auto_eoi;

    // Masking interrupts
    master_mask = 0xff;
    slave_mask = 0xff;

    volatile int i = 1;
    i++;
    // magically doesn't work if we don't do this!!

    // Start Initialize
    outb(ICW1, MASTER_8259_PORT);
    outb(ICW1, SLAVE_8259_PORT);
   
    
    // Vector offset
    outb(ICW2_MASTER, MASTER_8259_DATA);
    outb(ICW2_SLAVE, SLAVE_8259_DATA);
   

    // Identities
    outb(ICW3_MASTER, MASTER_8259_DATA);
    outb(ICW3_SLAVE, SLAVE_8259_DATA);
   

    // ICW4 Environment?
    outb(ICW4, MASTER_8259_DATA);
    outb(ICW4, SLAVE_8259_DATA);
   

    outb(master_mask, MASTER_8259_DATA);
    outb(slave_mask, SLAVE_8259_DATA);
   

    enable_irq(IRQ_SECONDARY_PIC);    

    // 8259 should be initialized

}

/* void enable_irq(uint32_t irq_num)
 * enables a specific IRQ
 * Inputs: irq_num - The IRQ to enable
 * Outputs: None
 * Side Effects: Enables the specified IRQ
 */
void enable_irq(uint32_t irq_num) {
    uint16_t port;
    uint16_t inum;
    
    if (irq_num > MAX_DEVICES)
        return;

    // Get which PIC
    if (irq_num >= PIC_DEVICES) {
        port = SLAVE_8259_DATA;
        inum = irq_num - PIC_DEVICES;
        slave_mask &= ~(1 << inum);
        outb(slave_mask, port);
    }
    else {
        port = MASTER_8259_DATA;
        inum = irq_num;
        master_mask &= ~(1 << inum);
        outb(master_mask, port);
    }
    
    return;
}

/* void disable_irq(uint32_t irq_num)
 * disables a specific IRQ
 * Inputs: irq_num - The IRQ to disable
 * Outputs: None
 * Side Effects: Disables the specified IRQ
 */
 void disable_irq(uint32_t irq_num) {
    uint16_t port;
    uint16_t inum;
    
    if (irq_num > MAX_DEVICES)
        return;

    // Get which PIC
    if (irq_num >= PIC_DEVICES) {
        port = SLAVE_8259_DATA;
        inum = irq_num - PIC_DEVICES;
        slave_mask |= (1 << inum);
        outb(slave_mask, port);
    }
    else {
        port = MASTER_8259_DATA;
        inum = irq_num;
        master_mask |= (1 << inum);
        outb(master_mask, port);
    }
    
    return;
}


/* void send_eoi(uint32_t irq_num)
 * sends EOI to a specific IRQ
 * Inputs: irq_num - The IRQ to send EOI to
 * Outputs: None
 * Side Effects: Sends EOI to the IRQ
 */
 void send_eoi(uint32_t irq_num) {
    if (irq_num > MAX_DEVICES)
        return;
    // If Secondary, send EOI to both PICS
    if(irq_num >= PIC_DEVICES) {
        outb((EOI|(irq_num-PIC_DEVICES)), SLAVE_8259_PORT);
        outb((EOI|0x2), MASTER_8259_PORT);
    }
    else{
        outb((EOI|irq_num), MASTER_8259_PORT);
    }
}
