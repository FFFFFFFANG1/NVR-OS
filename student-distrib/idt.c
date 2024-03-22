#include "idt.h"
#include "x86_desc.h"
#include "lib.h"
#include "idt_linkage.h"

/*
division_error
    DESCRIPTION: exception[0x00]
    INPUT: none
    OUTPUT: exception print sentense
    SIDE EFFECT: print the error out
*/
void division_error_exception(){
    clear();
    printf("Divide by 0!\n");
    halt(255);
}

/*
debug_exception
    DESCRIPTION: exception[0x01]
    INPUT: none
    OUTPUT: exception print sentense
    SIDE EFFECT: print the error out
*/
void debug_exception(){
    clear();
    printf("Debug!\n");
    halt(255);
}

/*
nonmaskable_interrupt_exception
    DESCRIPTION: exception[0x02]
    INPUT: none
    OUTPUT: exception print sentense
    SIDE EFFECT: print the error out
*/
void nonmaskable_interrupt_exception(){
    clear();
    printf("Non-maskable Interrupt!\n");
    halt(255);
}

/*
breakpoint_exception
    DESCRIPTION: exception[0x03]
    INPUT: none
    OUTPUT: exception print sentense
    SIDE EFFECT: print the error out
*/
void breakpoint_exception(){
    clear();
    printf("Breakpoint!\n");
    halt(255);
}

/*
overflow_exception
    DESCRIPTION: exception[0x04]
    INPUT: none
    OUTPUT: exception print sentense
    SIDE EFFECT: print the error out
*/
void overflow_exception(){
    clear();
    printf("Overflow!\n");
    halt(255);
}

/*
bound_range_exceeded_exception
    DESCRIPTION: exception[0x05]
    INPUT: none
    OUTPUT: exception print sentense
    SIDE EFFECT: print the error out
*/
void bound_range_exceeded_exception(){
    clear();
    printf("Bound Range Exceeded!\n");
    halt(255);
}

/*
invalid_opcode_exception
    DESCRIPTION: exception[0x06]
    INPUT: none
    OUTPUT: exception print sentense
    SIDE EFFECT: print the error out
*/
void invalid_opcode_exception(){
    clear();
    printf("Invalid Opcode!\n");
    halt(255);
}

/*
device_not_available_exception
    DESCRIPTION: exception[0x07]
    INPUT: none
    OUTPUT: exception print sentense
    SIDE EFFECT: print the error out
*/
void device_not_available_exception(){
    clear();
    printf("Device Not available!\n");
    halt(255);
}

/*
double_fault_exception
    DESCRIPTION: exception[0x08]
    INPUT: none
    OUTPUT: exception print sentense
    SIDE EFFECT: print the error out
*/
void double_fault_exception(){
    clear();
    printf("Double Fault!\n");
    halt(255);
}

/*
coprocessor_segment_overrun_exception
    DESCRIPTION: exception[0x09]
    INPUT: none
    OUTPUT: exception print sentense
    SIDE EFFECT: print the error out
*/
void coprocessor_segment_overrun_exception(){
    clear();
    printf("Coprocessor Segment Overrun!\n");
    halt(255);
}

/*
invalid_tss_exception
    DESCRIPTION: exception[0x0A]
    INPUT: none
    OUTPUT: exception print sentense
    SIDE EFFECT: print the error out
*/
void invalid_tss_exception(){
    clear();
    printf("Invalid TSS!\n");
    halt(255);
}

/*
segment_not_present_exception
    DESCRIPTION: exception[0x0B]
    INPUT: none
    OUTPUT: exception print sentense
    SIDE EFFECT: print the error out
*/
void segment_not_present_exception(){
    clear();
    printf("Segment Not Present!\n");
    halt(255);
}

/*
stack_segment_fault_exception
    DESCRIPTION: exception[0x0C]
    INPUT: none 
    OUTPUT: exception print sentense
    SIDE EFFECT: print the error out
*/
void stack_segment_fault_exception(){
    clear();
    printf("Stack-Segment Fault\n");
    halt(255);
}

/*
general_protection_fault_exception
    DESCRIPTION: exception[0x0D]
    INPUT: none
    OUTPUT: exception print sentense
    SIDE EFFECT: print the error out
*/
void general_protection_fault_exception(){
    clear();
    printf("General Protection Fault!\n");
    halt(255);
}

/*
 * page_fault_exception
    *   DESCRIPTION: exception[0x0E]
    *  INPUT: none
    * OUTPUT: exception print sentense
    * SIDE EFFECT: print the error out
 */
void page_fault_exception(){
    clear();
    printf("Page Fault!\n");
    halt(255);
}

/*
 * x87_floating_point_exception
    *   DESCRIPTION: exception[0x10]
    *  INPUT: none
    * OUTPUT: exception print sentense
    * SIDE EFFECT: print the error out
 */
void x87_floating_point_exception(){
    clear();
    printf("x87 Floating-Point Exception!\n");
    halt(255);
}

/*
 * alignment_check_exception
    *   DESCRIPTION: exception[0x11]
    *  INPUT: none
    * OUTPUT: exception print sentense
    * SIDE EFFECT: print the error out
 */
void alignment_check_exception(){
    clear();
    printf("Alignment Check!\n");
    halt(255);
}

/*
 * machine_check_exception
    *   DESCRIPTION: exception[0x12]
    *  INPUT: none
    * OUTPUT: exception print sentense
    * SIDE EFFECT: print the error out
 */
void machine_check_exception(){
    clear();
    printf("Machine Check!\n");
    halt(255);
}

/*
 * simd_floating_point_exception
    *   DESCRIPTION: exception[0x13]
    *  INPUT: none
    * OUTPUT: exception print sentense
    * SIDE EFFECT: print the error out
 */
void simd_floating_point_exception(){
    clear();
    printf("SIMD Floating-Point Exception!\n");
    halt(255);
}

/*
init_idt
    DESCRIPTION: initialize the IDT, set all the bits to the required value
    INPUT: none
    OUTPUT: none
    SIDE EFFECT: initialize the IDT
*/
extern void init_idt(){
    int i;
    // init the exception part (0x00-0x13)
    for (i = 0x00; i <= 0x13; i++){
        if (i == 0x0F) continue; // idt[0x0F] is reserved by Intel
        idt[i].present = 1;
        idt[i].dpl = 0;
        idt[i].reserved1 = 1;
        idt[i].reserved2 = 1;
        idt[i].reserved3 = 1;
        idt[i].size = 1;
        idt[i].seg_selector = KERNEL_CS;
    }

    // init the interrupt part
    //0x20: pit handler
    idt[0x20].present = 1;
    idt[0x20].dpl = 0;
    idt[0x20].reserved1 = 1;
    idt[0x20].reserved2 = 1;
    idt[0x20].reserved3 = 0;
    idt[0x20].size = 1;
    idt[0x20].seg_selector = KERNEL_CS;
    // 0x21: keyboard handler
    idt[0x21].present = 1;
    idt[0x21].dpl = 0;
    idt[0x21].reserved1 = 1;
    idt[0x21].reserved2 = 1;
    idt[0x21].reserved3 = 0;
    idt[0x21].size = 1;
    idt[0x21].seg_selector = KERNEL_CS;
    // 0x28: RTC handler
    idt[0x28].present = 1;
    idt[0x28].dpl = 0;
    idt[0x28].reserved1 = 1;
    idt[0x28].reserved2 = 1;
    idt[0x28].reserved3 = 0;
    idt[0x28].size = 1;
    idt[0x28].seg_selector = KERNEL_CS;

    // init the system call part (0x80)
    idt[0x80].present = 1;
    idt[0x80].dpl = 3;
    idt[0x80].reserved1 = 1;
    idt[0x80].reserved2 = 1;
    idt[0x80].reserved3 = 1;
    idt[0x80].size = 1;
    idt[0x80].seg_selector = KERNEL_CS;

    // setting IDT entries
    SET_IDT_ENTRY(idt[0x00], division_error_linkage);
    SET_IDT_ENTRY(idt[0x01], debug_linkage);
    SET_IDT_ENTRY(idt[0x02], nonmaskable_interrupt_linkage);
    SET_IDT_ENTRY(idt[0x03], breakpoint_linkage);
    SET_IDT_ENTRY(idt[0x04], overflow_linkage);
    SET_IDT_ENTRY(idt[0x05], bound_range_exceeded_linkage);
    SET_IDT_ENTRY(idt[0x06], invalid_opcode_linkage);
    SET_IDT_ENTRY(idt[0x07], device_not_available_linkage);
    SET_IDT_ENTRY(idt[0x08], double_fault_linkage);
    SET_IDT_ENTRY(idt[0x09], coprocessor_segment_overrun_linkage);
    SET_IDT_ENTRY(idt[0x0A], invalid_tss_linkage);
    SET_IDT_ENTRY(idt[0x0B], segment_not_present_linkage);
    SET_IDT_ENTRY(idt[0x0C], stack_segment_fault_linkage);
    SET_IDT_ENTRY(idt[0x0D], general_protection_fault_linkage);
    SET_IDT_ENTRY(idt[0x0E], page_fault_linkage);
    SET_IDT_ENTRY(idt[0x10], x87_floating_point_linkage);
    SET_IDT_ENTRY(idt[0x11], alignment_check_linkage);
    SET_IDT_ENTRY(idt[0x12], machine_check_linkage);
    SET_IDT_ENTRY(idt[0x13], simd_floating_point_linkage);

    SET_IDT_ENTRY(idt[0x20],pit_handler_linkage);
    SET_IDT_ENTRY(idt[0x21], keyboard_handler_linkage);
    SET_IDT_ENTRY(idt[0x28], rtc_handler_linkage);

    SET_IDT_ENTRY(idt[0x80],systemcall_linkage);

    // loading IDT
    lidt(idt_desc_ptr);
}
