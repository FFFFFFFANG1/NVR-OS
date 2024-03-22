#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"
#include "terminal.h"


#define PAGE_DIRECTORY_SIZE 1024
#define PAGE_TABLE_SIZE 1024
#define PAGE_SIZE 4096
#define PTE_addr_shift 12 // 4KB align
#define PDE_addr_shift 22 // 4MB align

//some addresses
#define kernel_start 0x400000
#define video_mem_orig 0xB8000
#define VISUAL_user_start 0x8000000 //128MB
#define user_prog_size 0x400000 //4MB
#define PHYSICAL_user_start 0x800000 //8MB
#define VISUAL_VIDMAP 0x8800000 //132 + 4MB for safety
#define page_size 0x1000 //4KB

//some pages status
#define init_dict_entry 0x00000002 //supervisor, write, not present
#define attr_rw 0x00000002 
#define attr_present 0x00000001 
#define attr_user 0x00000004

/* init the paging */
void init_paging();
/* use the pid to map a user level program to physical memory*/
extern void user_prog_paging(uint32_t pid);
/*set the video mapping page after 132MB user virtual paging*/
extern int32_t vidmap_paging();
/*video memory mapping for multi-terminal*/
extern void multi_vidmem_mapping(uint32_t terminal_idx);
extern void multi_vidmap_update();

#endif
