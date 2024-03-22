#define ASM 


#define ASM_LINK(name,func)     \
    .globl name                ;\
    name:                      ;\
        pushal                 ;\
        pushfl                 ;\
        call func              ;\
        popfl                  ;\
        popal                  ;\
        iret

# deal with the interrupt
ASM_LINK(keyboard_handler_linkage,keyboard_input_handler)
ASM_LINK(rtc_handler_linkage,rtc_handler)
ASM_LINK(pit_handler_linkage,pit_handler)

# deal with the system call
.globl systemcall_table
systemcall_table:
    .long 0     # align for the system call to be 1-15s
    .long halt
    .long execute
    .long read
    .long write
    .long open
    .long close
    .long getargs
    .long vidmap
    .long set_handler
    .long sigreturn
    .long ps
    .long file_create
    .long file_delete
    .long file_copy
    .long color

.globl systemcall_linkage

systemcall_linkage:
    pushfl
    pushl %ebp   # push all registers except eax
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx
    
    cmpl $1, %eax # check the system call number
    jl invalid_systemcall
    cmpl $15, %eax
    ja invalid_systemcall

    pushl %edx
    pushl %ecx
    pushl %ebx
    call *systemcall_table(, %eax, 4)
    popl %ebx
    popl %ecx
    popl %edx
    jmp DONE

invalid_systemcall:
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp
    popfl
    movl $-1, %eax
    iret

DONE:
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp # exit with the return value of the system call number in eax
    popfl
    iret

# deal with the exception
ASM_LINK(division_error_linkage,division_error_exception)
ASM_LINK(debug_linkage,debug_exception)
ASM_LINK(nonmaskable_interrupt_linkage,nonmaskable_interrupt_exception)
ASM_LINK(breakpoint_linkage,breakpoint_exception)
ASM_LINK(overflow_linkage,overflow_exception)
ASM_LINK(bound_range_exceeded_linkage,bound_range_exceeded_exception)
ASM_LINK(invalid_opcode_linkage,invalid_opcode_exception)
ASM_LINK(device_not_available_linkage,device_not_available_exception)
ASM_LINK(double_fault_linkage,double_fault_exception)
ASM_LINK(coprocessor_segment_overrun_linkage,coprocessor_segment_overrun_exception)
ASM_LINK(invalid_tss_linkage,invalid_tss_exception)
ASM_LINK(segment_not_present_linkage,segment_not_present_exception)
ASM_LINK(stack_segment_fault_linkage,stack_segment_fault_exception)
ASM_LINK(general_protection_fault_linkage,general_protection_fault_exception)
ASM_LINK(page_fault_linkage,page_fault_exception)
ASM_LINK(x87_floating_point_linkage,x87_floating_point_exception)
ASM_LINK(alignment_check_linkage,alignment_check_exception)
ASM_LINK(machine_check_linkage,machine_check_exception)
ASM_LINK(simd_floating_point_linkage,simd_floating_point_exception)
