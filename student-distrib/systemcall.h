#ifndef SYSTEMCALL_H
#define SYSTEMCALL_H

#include "lib.h"
#include "types.h"
#include "x86_desc.h"
#include "file_system.h"
#include "rtc.h"
#include "idt_linkage.h"
#include "paging.h"


// extern void systemcall_handler();

#define PCB_GET_MASK 0xFFFFE000
#define BAMB     0x800000
#define BAKB     0x002000
#define MAX_FILE_NUMBER      8
#define MAX_PROCESS_NUMBER   6 

// file operation table
typedef struct file_operation{
    int32_t (*open)(const uint8_t* filename);
    int32_t (*close)(int32_t fd);
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
}fop_table_t;

// file descriptor structure
typedef struct file_decs{
    fop_table_t op_table;
    uint32_t inode;
    uint32_t file_position;
    uint32_t flags; /*0 for free, 1 for occupied*/
}file_decs_t;

// pcb structure
typedef struct pcb{
    uint32_t pid;
    uint32_t parent_pid;
    file_decs_t file_decs[8]; /*max number of files is 8*/
    uint32_t saved_esp;
    uint32_t saved_ebp;
    uint32_t parent_esp;
    uint32_t parent_ebp;
    uint32_t prog_start_addr;
    int8_t arg[128]; // the max length for input is 128 in shell
    int32_t arg_len; // the length of argument
    int8_t process_name[FILENAME_LEN]; // the max length for process name is 32
}pcb_t;

// some helper functions for getting pcb
extern pcb_t* get_pcb(uint32_t pid);
extern pcb_t* get_cur_pcb();
pcb_t* get_parent_pcb();


// function called by the corresponding system call
int32_t halt(uint8_t status);
int32_t execute(const uint8_t* command);
int32_t execute_shell(const uint8_t* command);
int32_t read(int32_t fd, void* buf, int32_t nbytes);
int32_t write(int32_t fd, const void* buf, int32_t nbytes);
int32_t open(const uint8_t* filename);
int32_t close(int32_t fd);
int32_t getargs(uint8_t* buf, int32_t nbytes);
int32_t vidmap(uint8_t** screen_start);
int32_t set_handler(int32_t signum, void* handler_address);
int32_t sigreturn(void);
//for extra credit
int32_t ps(void);
uint32_t ancestor_pid(uint32_t pid);
int32_t color(uint8_t text_color, uint8_t background_color);

#endif
