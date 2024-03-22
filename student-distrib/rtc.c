#include "lib.h"
#include "i8259.h"
#include "rtc.h"


// int32_t device_frequency = 2; //initialize the virtualized rtc frequency to 2
int32_t device_freq[6]; //initialize the virtualized rtc frequency to 2
volatile int32_t rtc_flag[6]; //1 means interrupt should happen, 0 mean waiting
volatile int32_t rtc_counter = 0; //record how many times real rtc interrupt occurs (handler is called)
int32_t eightMB = 0x800000;
int32_t eightKB = 0x002000;

/* rtc_init()
 * Inputs: none
 * Return Value: none
 * Function: Initialize the real-time clock, enable RTC interrupt.
 */
void rtc_init () {
    char prev;
    int rate = 0;//frequency =  32768 >> (rate-1), default frequency = 2 Hz
    int i;
    /* first, turn on the periodic interrupt */
    cli();
    outb(reg_b, rtc_index);//select reg_b with NMI disabled
    prev = inb(rtc_data);//read cur value in reg_b
    outb(reg_b, rtc_index);//reset reg index to reg_b
    //0x40 ORed with prev value, which means bit 6 is set to 1 and the periodic interrupt is turned on
    outb((0x40 | prev), rtc_data);
    sti();

    /* second, set the rate of the frequency */
    rate |= 6;//rate is ranging from 3 to 15, rate = 6, frequency =  32768 >> (rate-1), default frequency = 1024Hz
    cli();
    outb(reg_a, rtc_index);
    prev = inb(rtc_data);//store the prev  value of reg_a, whose lowest 4 bits represent the rate
    outb(reg_a, rtc_index);//reset reg_index to reg_a
    //keep highest 4 bits of the reg_a while setting the rate, default frequency = 2 Hz
    outb( (0xF0 & prev) | rate, rtc_data);//0xF0 is bitmask of highest 4 bits of reg_a byte
    sti();

    /* third, turn on IRQ 8 */
    enable_irq(8);
    
    /* finally, make sure all RTC interrupts that were being handled while or before rtc initialization are acknowledged */
    /* if not do so, the RTC may not send any interrupts after rebooting */
    outb(reg_c, rtc_index);//select reg C
	inb(rtc_data);//read reg C after an IRQ8 to ensure that the interrupt can happen again

    /* ADDING IN CHECKPOINT 2 */
    // initialize global variables
    // device_frequency = 2;//set the  virtualized frequency to 2
    // rtc_flag = 0;//1 means interrupt should happen, 0 mean waiting

    rtc_counter = 0;//record how many times real rtc interrupt occurs (handler is called)
    for (i = 0; i < 6; i++) {
        device_freq[i] = 16;//set the virtualized frequency to 16
        rtc_flag[i] = 0;//1 means interrupt should happen, 0 mean waiting
    }
    return;//done
}

/* rtc_handler()
 * Inputs: none
 * Return Value: none
 * Function: set up the RTC handler and send "end of interrupt" signal. Set flag to 1 if an interrupt should occur.
 */
void rtc_handler () {
    cli();
    int i;
    //After "interrupt_interval" times of rtc interrupt, return back to the process
    int32_t current_pid;
    int32_t cur_esp;
    asm volatile("movl %%esp, %0" : "=r"(cur_esp));
    current_pid = (eightMB - cur_esp) / eightKB;
    rtc_counter++;//increment counter when the handler is called
    //set rtc interrupt flag if exact one interrupt interval is passed
    for (i = 0; i < 6; i++) {
        if (rtc_counter % (real_frequency / device_freq[i]) == 0) {
            rtc_flag[i] = 1;//set the flag to 1 if an interrupt should occur
        }
    }
    // printf("rtc_handler can use\n");
    outb(0x0C, rtc_index);//select reg C
    inb(rtc_data);//read reg C after an IRQ8 to ensure that the interrupt can happen again 
    send_eoi(8);//turn off IRQ 8
    sti();
    
    return;//done
}

/* rtc_open()
 * Inputs: filename (ignored)
 * Return Value: 0 for success
 * Function: reset the frequency to 2Hz and clear the rtc flag.
 */
int32_t rtc_open (const uint8_t* filename) {
    int32_t current_pid;
    int32_t cur_esp;
    asm volatile("movl %%esp, %0" : "=r"(cur_esp));
    current_pid = (eightMB - cur_esp) / eightKB;
    rtc_flag[current_pid] = 0;// clear the rtc flag, step in a new process
    device_freq[current_pid] = 2;//reset the frequency to 2
    
    return 0;//success, return 0
}

/* rtc_read()
 * Inputs: fd -- ignored; buf -- ignored; nbytes -- size of 'buf' (unit: byte) (ignored)
 * Return Value: 0 for success
 * Function: Wait until an rtc interrupt occurs (flag set to 1). And clear the flag after that.
 */
int32_t rtc_read (int32_t fd, void* buf, int32_t nbytes) {
    int32_t current_pid;
    int32_t cur_esp;
    asm volatile("movl %%esp, %0" : "=r"(cur_esp));
    current_pid = (eightMB - cur_esp) / eightKB;
    while (1) {//keep waiting until the flag is set and it's ready to return to the process
        if (1 == rtc_flag[current_pid]) break;//exit the loop when flag is set to 1
    }
    rtc_flag[current_pid] = 0;//clear the flag after exit the spin loop
    
    return 0;//success, return 0
}

/* rtc_close()
 * Inputs: none
 * Return Value: 0 for success
 * Function: do nothing and return 0 for success
 */
int32_t rtc_close (int32_t fd) {
    return 0;//success, return 0
}

/* rtc_write()
 * Inputs: fd -- ignored; buf -- desired frequency for devices; nbytes -- size of 'buf' (unit: byte) (ignored)
 * Return Value: 0 for success, -1 for failure
 * Function: Set a new frequency if the desired frequency is valid.
 */
int32_t rtc_write (int32_t fd, const void* buf, int32_t nbytes) {
    int32_t current_pid;
    int32_t cur_esp;
    asm volatile("movl %%esp, %0" : "=r"(cur_esp));
    current_pid = (eightMB - cur_esp) / eightKB;
    int32_t frequency_desired = *(int32_t*) buf;//get the desired input (desired frequency for the specific process)
    // the front two conditions are to check if the frequency is a power of two
    // the last two are to check the range
    if ((frequency_desired > 0) && ((frequency_desired & (frequency_desired - 1)) == 0) && frequency_desired <= real_frequency && frequency_desired >= 2){
        device_freq[current_pid] = frequency_desired;//set the virtualized frequency
        //  printf("rtc_write success\n");
        return 0;//return 0 if frequency set is successful
    } else {
        return -1;//return -1 if frequency set fails
    }

    return 0;//success, return 0
}
