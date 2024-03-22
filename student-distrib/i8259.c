/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/*
*   Function: i8259_init()
*   Decription: Initialize the 8259 PIC
*   Input: None
*   Output: None
*   Return value: None
*   Side effect: PICs will be initialized
*/
void i8259_init(void) {
    /* Mask all the interrupts */
    outb(MASK_ALL_INTERRUPTS,MASTER_8259_DATA_PORT);
    outb(MASK_ALL_INTERRUPTS,SLAVE_8259_DATA_PORT);

    /* Initialize Mastr PIC */
    outb(ICW1,MASTER_8259_COMMAND_PORT);
    outb(ICW2_MASTER,MASTER_8259_DATA_PORT);
    outb(ICW3_MASTER,MASTER_8259_DATA_PORT);
    outb(ICW4,MASTER_8259_DATA_PORT);

    /* Initialize Slave PIC */
    outb(ICW1,SLAVE_8259_COMMAND_PORT);
    outb(ICW2_SLAVE,SLAVE_8259_DATA_PORT);
    outb(ICW3_SLAVE,SLAVE_8259_DATA_PORT);
    outb(ICW4,SLAVE_8259_DATA_PORT);


    master_mask = 0xFF;
    slave_mask = 0xFF;
    enable_irq(2); // slave PIC is connected to IR2 in master PIC

}

/*
*   Function: enable_irq
*   Decription: unmask certain IRQ in primary PIC or slave PIC
*   Input: irq_num -- irq will be unmasked
*   Output: None
*   Return value: None
*   Side effect: One irq can receive interrupt
*/
void enable_irq(uint32_t irq_num) {
    unsigned int base_condition = 0x01;
    unsigned int new_mask;

    /* first, do sanity check */
    if( irq_num < 0 || irq_num > 15 ){
        return;
    }


    /* if irq_num is on master PIC*/
    if (irq_num >= 0 && irq_num <= 7){
        new_mask = base_condition << irq_num;
        new_mask = ~new_mask;
        master_mask = master_mask & new_mask;  // set corresponding bit to 0 to unmask interrupt
        outb(master_mask, MASTER_8259_DATA_PORT);
    }

    /* if irq_num is on slave PIC*/
    if (irq_num >= 8 && irq_num <= 15){
        irq_num = irq_num - 8; // reset irq_num to make it more convenient

        new_mask = base_condition << irq_num;
        new_mask = ~new_mask;
        slave_mask = slave_mask & new_mask;  // set corresponding bit to 0 to unmask interrupt
        outb(slave_mask, SLAVE_8259_DATA_PORT);
    }

}

/*
*   Function: disable_irq
*   Description: mask the specified IRQ
*   Input: irq_num -- irq will be masked
*   Output: None
*   Return value: None
*   Side effect: One irq can not receive interrupt
*/
void disable_irq(uint32_t irq_num) {
    unsigned int base_condition = 0x01;
    unsigned int new_mask;

    /* first, do sanity check */
    if( irq_num < 0 || irq_num > 15 ){
        return;
    }

    /* if irq_num is on master PIC*/
    if (irq_num >= 0 && irq_num <= 7){
        new_mask = base_condition << irq_num;
        master_mask = master_mask | new_mask;  // set corresponding bit to 1 to mask interrupt
        outb(master_mask, MASTER_8259_DATA_PORT);
    }

    /* if irq_num is on slave PIC*/
    if (irq_num >= 8 && irq_num <= 15){
        irq_num = irq_num - 8; // reset irq_num to make it more convenient

        new_mask = base_condition << irq_num;
        slave_mask = slave_mask | new_mask;  // set corresponding bit to 1 to mask interrupt
        outb(slave_mask, SLAVE_8259_DATA_PORT);
    }

}

/*
*   Function: send_eoi
*   Description: Send EOI to certain IRQ
*   Input: irq_num -- IRQ has finished
*   Output: None
*   Return value: None
*   Side effect: IRQ receives EOI and allows the interrupt happen again
*/
void send_eoi(uint32_t irq_num) {
    unsigned int new_EOI;

    /* first, do sanity check */
    if( irq_num < 0 || irq_num > 15 ){
        return;
    }

    /* if irq_num is in master PIC*/
    if ( irq_num >= 0 && irq_num <= 7 ){
        new_EOI = EOI | irq_num;
        outb(new_EOI, MASTER_8259_COMMAND_PORT);
    }

    /* if irq_num is in slave PIC*/
    if ( irq_num >= 8 && irq_num <= 15 ){
        irq_num = irq_num - 8;
        new_EOI = EOI | 2;  // slave PIC is connected to IR2 in the primary PIC

        outb(EOI | irq_num, SLAVE_8259_COMMAND_PORT); // first we should send EOI to slave PIC
        outb(new_EOI, MASTER_8259_COMMAND_PORT);  // then send EOI to master PIC       
    }
 
}
