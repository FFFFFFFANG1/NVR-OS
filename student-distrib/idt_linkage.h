#ifndef IDT_LNKAGE_H
#define IDT_LNKAGE_H

#include "systemcall.h"
#include "rtc.h"
#include "keyboard.h"
#include "scheduler.h"

#ifndef ASM

// the interrupt linkage
extern void keyboard_handler_linkage();
extern void rtc_handler_linkage();
extern void pit_handler_linkage();

// the system call linkage
extern void systemcall_linkage();

// the exception linkage
extern void division_error_linkage();
extern void debug_linkage();
extern void nonmaskable_interrupt_linkage();
extern void breakpoint_linkage();
extern void overflow_linkage();
extern void bound_range_exceeded_linkage();
extern void invalid_opcode_linkage();
extern void device_not_available_linkage();
extern void double_fault_linkage();
extern void coprocessor_segment_overrun_linkage();
extern void invalid_tss_linkage();
extern void segment_not_present_linkage();
extern void stack_segment_fault_linkage();
extern void general_protection_fault_linkage();
extern void page_fault_linkage();
extern void x87_floating_point_linkage();
extern void alignment_check_linkage();
extern void machine_check_linkage();
extern void simd_floating_point_linkage();

#endif
#endif
