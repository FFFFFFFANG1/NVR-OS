#include "scheduler.h"
#include "systemcall.h"

volatile int32_t cur_index;
volatile int32_t SCHED_TASKS[SCHED_TOTAL];

/*
    * pit_init
    *   DESCRIPTION: initialize the pit
    *  INPUTS: none
    * OUTPUTS: none
    *  RETURN VALUE: none
    * SIDE EFFECTS: none
 */
void pit_init(){
    // the divisor for our pit (frequency)
    int divisor = PIT_FREQ_DEFAULT / PIT_COUNTER;
    int low_byte = divisor & 0xFF;
    int high_byte = divisor >> 8;
    // choose mode and write data to it
    outb(PIT_MODE, PIT_MODE_PORT);
    outb(low_byte, PIT_DATA_PORT);
    outb(high_byte, PIT_DATA_PORT);
    enable_irq(PIT_IRQ);
    return;
}

/*
    * pit_handler
    *   DESCRIPTION: handle the pit interrupt
    *  INPUTS: none
    * OUTPUTS: none
    *  RETURN VALUE: none
    * SIDE EFFECTS: none
 */
void pit_handler(){
    // critical section 
    // printf("pit handler\n");
    // cli();
    // send eoi
    send_eoi(PIT_IRQ);
    // scheduler start!
    // printf("before enter scheduler!\n");
    scheduler();
    // sti();
    return;
}

/*
    * scheduler_init
    *   DESCRIPTION: initialize the scheduler
    *  INPUTS: none
    * OUTPUTS: none
    *  RETURN VALUE: none
    * SIDE EFFECTS: none
 */
extern void scheduler_init(){
    int i;
    // init the variable
    cur_index = 0;
    for (i = 0; i < SCHED_TOTAL; i++){
        SCHED_TASKS[i] = -2;
    }
    return;
}

/*
    * scheduler
    *   DESCRIPTION: scheduler for the system
    *  INPUTS: none
    * OUTPUTS: none
    *  RETURN VALUE: none
    * SIDE EFFECTS: none
 */
void scheduler(){
    int ebp,esp;
    /*first save the ebp,esp and store it to the pcb of corresponding process*/
    asm volatile ("          \n\
                 movl %%ebp, %0  \n\
                 movl %%esp, %1  \n\
            "
            :"=r"(ebp), "=r"(esp)
            );
    if (SCHED_TASKS[cur_index] != -2){
        pcb_t* cur_pcb = get_pcb(SCHED_TASKS[cur_index]);
        cur_pcb->saved_ebp = ebp;
        cur_pcb->saved_esp = esp;
    }
    if (SCHED_TASKS[cur_index] == -2){
        /* remap the video memory for this shell!*/
        multi_vidmem_mapping(cur_index);
        // printf("make a new shell\n");
        printf("Terminal %d\n", cur_index);
        execute_shell((uint8_t*)"shell"); 
        
    }
    // go to next terminal
    cur_index = (cur_index + 1) % SCHED_TOTAL;
    // terminal has no shell
    if (SCHED_TASKS[cur_index] == -2){
        /* remap the video memory for this shell!*/
        multi_vidmem_mapping(cur_index);
        printf("Terminal %d\n", cur_index);
        // printf("make a new shell\n");
        execute_shell((uint8_t*)"shell");
    }
    
    // get the pcb we need
    pcb_t* next_pcb = get_pcb(SCHED_TASKS[cur_index]);
    // point user program to the physical memory
    user_prog_paging(SCHED_TASKS[cur_index]);

    /* change TSS*/
    tss.ss0 = KERNEL_DS;
    tss.esp0 = BAMB - SCHED_TASKS[cur_index] * BAKB - sizeof(int32_t); // subtract 4 for dummy bits

    /* remap physical memory ()*/ 
    multi_vidmem_mapping(cur_index);
    sti();
    
    /* context switch*/
    asm volatile (
            "movl %0, %%ebp  \n"
            "movl %1, %%esp  \n"
            "leave          \n\
             ret            \n"
            :
            : "r"(next_pcb->saved_ebp), "r"(next_pcb->saved_esp)
        );
}
