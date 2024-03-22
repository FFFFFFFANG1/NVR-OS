#include "paging.h"

// Global page directory
uint32_t page_directory[PAGE_DIRECTORY_SIZE] __attribute__((aligned(PAGE_SIZE)));
// Global page table
uint32_t page_table[PAGE_TABLE_SIZE] __attribute__((aligned(PAGE_SIZE)));
// page table for video memory
uint32_t video_page_table[PAGE_TABLE_SIZE] __attribute__((aligned(PAGE_SIZE)));

//calculate the address of three backup screens, started from 8kb from the physical screen. Left 4kb as buffer.s
uint32_t SCREEN_1 = video_mem_orig + PAGE_SIZE * 2;
uint32_t SCREEN_2 = video_mem_orig + PAGE_SIZE * 3;
uint32_t SCREEN_3 = video_mem_orig + PAGE_SIZE * 4;
/*
 * init_paging
    * DESCRIPTION: Initialize paging
    * INPUTS: none
    * OUTPUTS: none
    * RETURN VALUE: none
    * SIDE EFFECTS: Enable paging
 */
void init_paging()
{
    uint16_t i;
    for (i = 0; i < PAGE_DIRECTORY_SIZE; i++) {
        page_directory[i] = init_dict_entry;
    }

    uint32_t video_addr = video_mem_orig >> PTE_addr_shift;
    uint32_t screen_1_addr = SCREEN_1 >> PTE_addr_shift;
    uint32_t screen_2_addr = SCREEN_2 >> PTE_addr_shift;
    uint32_t screen_3_addr = SCREEN_3 >> PTE_addr_shift;

    for (i = 0; i < PAGE_TABLE_SIZE; i++) {   
        if (i == video_addr) {
            // Set one 4kb page to video memory
            page_table[i] = (i << PTE_addr_shift) | attr_rw | attr_present;
        } else if (i == screen_1_addr || i == screen_2_addr || i == screen_3_addr) {
            // Set one 4kb page to backup screen
            page_table[i] = (i << PTE_addr_shift) | attr_rw | attr_present; 
        } else {
            page_table[i] = (i << PTE_addr_shift) | attr_rw;
        }
    }

    page_directory[0] = ((uint32_t)page_table) | attr_rw | attr_present;
    page_directory[1] = kernel_start | attr_rw | attr_present | 0x80; //0x80 stands for PS, PS=1 is 4MB

    // Enable paging
    asm volatile(
        "movl %0, %%eax;"
        "movl %%eax, %%cr3;"
        "movl %%cr4, %%eax;"
        "orl $0x00000010, %%eax;"
        "movl %%eax, %%cr4;"
        "movl %%cr0, %%eax;"
        "orl $0x80000000, %%eax;"
        "movl %%eax, %%cr0;"
        :
        : "r"(page_directory)
        : "eax");
}

/*
* user_prog_paging
*   DESCRIPTION: Set paging for user program
*   INPUTS: pid - process id
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: Set paging for user program and flush TLB
*/
void user_prog_paging(uint32_t pid)
{
    // offset = 32
    uint32_t offset = VISUAL_user_start >> PDE_addr_shift; 
    page_directory[offset] = (PHYSICAL_user_start + pid * user_prog_size) | attr_rw | attr_present | attr_user | 0x80; //0x80 stands for PS, PS=1 is 4MB
    /*update cr3 to flush TLB*/
    asm volatile(
        "movl %%cr3, %%eax;"
        "movl %%eax, %%cr3;"
        :
        :
        : "eax");
}

/*
 * vidmap_paging
    * DESCRIPTION: Set paging for vidmap
    * INPUTS: none
    * OUTPUTS: none
    * RETURN VALUE: none
    * SIDE EFFECTS: Set paging for vidmap and flush TLB
 */
int32_t vidmap_paging()
{
    uint32_t i;
    uint32_t offset = VISUAL_VIDMAP >> PDE_addr_shift;
    video_page_table[0] = (uint32_t)(video_mem_orig) | attr_rw | attr_present | attr_user;
    // fill in the rest of the page table
    for (i = 1; i < PAGE_TABLE_SIZE; i++) {
        video_page_table[i] = (i << PTE_addr_shift) | attr_rw | attr_user;
    }
    page_directory[offset] = ((uint32_t)video_page_table) | attr_rw | attr_present | attr_user;
    /*update cr3 to flush TLB*/
    asm volatile(
        "movl %%cr3, %%eax;"
        "movl %%eax, %%cr3;"
        :
        :
        : "eax");
    return VISUAL_VIDMAP; // return the start address of video memory
}

/*
 * multi_vidmem_mapping
    * DESCRIPTION: Set paging for multi-terminal
    * INPUTS: terminal_idx - the index of the terminal that we are going to switch to
    * OUTPUTS: none
    * RETURN VALUE: none
    * SIDE EFFECTS: Set paging for multi-terminal and flush TLB
 */
void multi_vidmem_mapping(uint32_t terminal_idx){  
    // if the terminal is being displayed
    if (terminal_idx == current_terminal_index){
        // map virtual 0xb8000 to physical 0xb8000
        page_table[video_mem_orig >> PTE_addr_shift] = (video_mem_orig) | attr_rw | attr_present;
        // video_page_table[VISUAL_VIDMAP >> PTE_addr_shift] = (video_mem_orig) | attr_rw | attr_present | attr_user;
        video_page_table[0] = (video_mem_orig) | attr_rw | attr_present | attr_user;
    } else {
        // map virtual 0xb8000 to physical backup screen
        // +2 since we map idx 0 to *2 location
        page_table[video_mem_orig >> PTE_addr_shift] = (video_mem_orig + PAGE_SIZE * (terminal_idx + 2)) | attr_rw | attr_present;
        video_page_table[0] = (video_mem_orig + PAGE_SIZE * (terminal_idx + 2)) | attr_rw | attr_present | attr_user;
    }

    /*update cr3 to flush TLB*/
    asm volatile(
        "movl %%cr3, %%eax;"
        "movl %%eax, %%cr3;"
        :
        :
        : "eax");
}


/*
 * multi_vidmap_update
    * DESCRIPTION: Used for copyting pages between terminals
    * INPUTS: none
    * OUTPUTS: none
    * RETURN VALUE: none
    * SIDE EFFECTS: Update the paging for multi-terminal and flush TLB
 */
void multi_vidmap_update(){
    page_table[video_mem_orig >> PTE_addr_shift] = (video_mem_orig) | attr_rw | attr_present;
    /*update cr3 to flush TLB*/
    asm volatile(
        "movl %%cr3, %%eax;"
        "movl %%eax, %%cr3;"
        :
        :
        : "eax");
}

