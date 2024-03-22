#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "types.h"
#include "lib.h"
#include "i8259.h"
#include "paging.h"
#include "x86_desc.h"
#include "terminal.h"


// scheduler constants
//excludes 3 shells on boot
#define SCHED_TOTAL 3 
#define TASK_IDLE -1
// #define TASK_SHELL 13

// pit constants
#define PIT_IRQ         0 
// 0x36 is the Channel 0
#define PIT_MODE        0x36 
// 0x40 is the Channel 0 data port
#define PIT_DATA_PORT   0x40 
// 0x43 is the Mode/Command register (write only, a read is ignored)
#define PIT_MODE_PORT   0x43 
// defalut frequency of pit
#define PIT_FREQ_DEFAULT 1193180 
// the counter for pit
#define PIT_COUNTER 100

// variable need to use in systemcall.c
// save the pid of process being running on the terminal#
extern volatile int32_t SCHED_TASKS[SCHED_TOTAL];
// current index for the terminal
extern volatile int32_t cur_index;

// function we need to use
void scheduler();

extern void scheduler_init();

extern void pit_init();

extern void pit_handler();


#endif
