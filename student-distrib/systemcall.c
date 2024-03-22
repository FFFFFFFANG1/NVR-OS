#include "systemcall.h"
#include "scheduler.h"


// #define BAMB 0x800000
// #define BAKB 0x2000
#define PID_NULL 0xFFFFFFFF
// the process number
static uint32_t pid_counter = 0;

int32_t pid_array[MAX_PROCESS_NUMBER] = {0, 0, 0, 0, 0, 0};

/*
    * get_pcb
    *   DESCRIPTION: get the pcb using pid
    *  INPUT: pid
    * OUTPUT: None
    * RETURN VALUE: the current pcb
    * SIDE EFFECT: None
*/
extern pcb_t* get_pcb(uint32_t pid){
    return (pcb_t*) (BAMB - (pid+1) * BAKB);
}

extern pcb_t* get_cur_pcb(){
    return (pcb_t*) (BAMB - pid_counter * BAKB);
}

// pcb_t* get_parent_pcb(){
//     if (pid_counter == 0){
//         return NULL;
//     }
//     return get_pcb(pid_counter);
// }

/*
*   Function: halt
*   Description: terminates a process, returning the speciffed value to its parent process. 
*   Input: status -- status of process that we need to return to parent process
*   Output: None
*   Return value: 0 for success, -1 for failure
*   Side effect: stop the process we specified， return a value to the parent execute system cal
*/
int32_t halt(uint8_t status){
    // the variable we need to use
    int i;
    uint32_t return_value;
    uint32_t parent_ebp;
    uint32_t parent_esp;
    uint32_t temp_pid;
    uint32_t current_pid;
    // get the pid and pcb
    int32_t esp;
    asm volatile ("          \n\
                 movl %%esp, %0  \n\
            "
            :"=r"(esp)
            );
    current_pid = (BAMB - esp) / BAKB;
    pcb_t* current_pcb = get_pcb(current_pid);

    // update the scheduler array's pid
    for(i = 0; i < SCHED_TOTAL; i++){
        if(SCHED_TASKS[i] == current_pid){
            SCHED_TASKS[i] = current_pcb->parent_pid;
            break;
        }
    }
    // empty the argument area
    for ( i = 0; i < current_pcb->arg_len; i++){
        current_pcb->arg[i] = 0;
    }

    // current_pid = current_pcb->pid;
    // if status is 255, then the call from exception, should return 256
    if (status == 255){                          
        return_value = 256;
    }
    else {return_value = status;}

    /* close all files before terminate the process*/
    for (i = 0; i < MAX_FILE_NUMBER; i++){
        close(i);  
    }

    pid_array[current_pcb->pid] = 0;

    // go back to parent pid
    temp_pid = current_pid; // save the process we terminate in case we need to reboot it
    // change to parent pid
    current_pid = current_pcb->parent_pid;
    parent_ebp = current_pcb->parent_ebp;
    parent_esp = current_pcb->parent_esp;

    pid_counter --;
    /* we should make sure that there's always a terminal works */
    if (current_pid == PID_NULL){
        execute_shell((uint8_t*)"shell");
    }

    /* do paging again, since the process has already changed */
    user_prog_paging( current_pid );        

    /* Update the Task Switch Segment (TSS) with updated SS0 and ESP0.  */
    tss.ss0 = KERNEL_DS;
    tss.esp0 = BAMB - current_pid * BAKB - sizeof(int32_t); //subtract 4 for the return value

    asm volatile("movl %0, %%eax \n\
                  movl %1, %%ebp \n\
                  movl %2, %%esp \n\
                  leave          \n\
                  ret            \n"
                : /* no output */
                : "r" (return_value), \
                  "r" (parent_ebp), \
                  "r" (parent_esp)
                : "eax", "ebp", "esp");

    return 0;
}
/*
    * execute
    * DESCRIPTION: execute the program
    * INPUT: command -- the command we need to execute
    * OUTPUT: None
    * RETURN VALUE: 0 for success, -1 for failure
    * SIDE EFFECT: Call the system call passed in the command
*/
int32_t execute(const uint8_t* command){
    /*check validity*/
    if (command == NULL || strlen((char*) command) == 0){
        printf("invalid command\n");
        return -1;
    }

    // if (pid_counter >= MAX_PROCESS_NUMBER){
    //     printf("too many process!\n");
    //     return -1;
    // }
    /*local variables*/
    int32_t i, j;
    uint8_t filename[FILENAME_LEN] = {0};
    dentry_t temp;
    int8_t arg[FILENAME_LEN * 4] = {0};
    int32_t arg_length = 0;
    uint8_t check_exe[4]; /*check the four magic numbers*/
    pcb_t* new_pcb;
    // pcb_t* parent_pcb;
    //uint32_t parent_pid;
    uint32_t cur_pid;
    uint8_t* prog_addr = (uint8_t*) 0x8048000;
    uint32_t file_size = VISUAL_user_start + user_prog_size - 0x8048000;

    /*parse cmd*/
    for (i = 0; i < FILENAME_LEN; i++){
        if (command[i] == ' ' || command[i] == '\0'){ //first word as filename
            break;
        }
        filename[i] = command[i];
    }

    /*parse arg*/
    while (command[i] == ' '){
        i++;
    }
    // printf("argument is:");
    for (j = 0; i < strlen((char*) command); i++, j++){
        if (command[i] == ' ') arg[j] = ' '; //end of one arg
        if (command[i] == '\0') { //end of all args
            arg[j] = '\0';
            arg_length ++;
            break;
        } 
        arg[j] = command[i];
        // printf("%c",arg[j]);
        arg_length ++;
    }
    // printf("\n");
    /*check if the file is executable*/
    
    if (read_dentry_by_name(filename, &temp) == -1){
        printf("invalid filename\n");
        return -1;
    }

    if (read_data(temp.inode_num, 0, check_exe, 4) == -1){
        printf("invalid file\n");
        return -1;
    }

    // magic number of exec
    if (check_exe[0] != 0x7f || check_exe[1] != 0x45 || check_exe[2] != 0x4c || check_exe[3] != 0x46){
        printf("file not executable\n");
        return -1;
    }
    
    /*paging*/
    for (i = 0; i < MAX_PROCESS_NUMBER; i++){
        if (pid_array[i] == 0){
            cur_pid = i;
            pid_array[i] = 1;
            break;
        }
    }
    if (i == 6){
        printf("too many process!\n");
        return -1;
    }
    user_prog_paging(cur_pid);
    
    /*load the file*/
    if (read_data(temp.inode_num, 0, prog_addr, file_size) == -1){
        return -1;
    }
    
    /*create new PCB*/
    new_pcb = (pcb_t*) (BAMB - (cur_pid + 1) * BAKB);
    new_pcb->pid = cur_pid;

    int32_t esp;
    asm volatile ("          \n\
                 movl %%esp, %0  \n\
            "
            :"=r"(esp)
            );
    new_pcb->parent_pid = (BAMB - esp) / BAKB;
    // parent_pcb = get_pcb(parent_pid);

    // if (parent_pcb == NULL){ // no parent process
    //     new_pcb->parent_pid = PID_NULL; // the biggest integer
    // } else {
    //     new_pcb->parent_pid = parent_pcb->pid;
    // }
    new_pcb->saved_esp = 0;
    new_pcb->saved_ebp = 0;
    // save the argment into the PCB
    new_pcb->arg_len = arg_length;
    memcpy(new_pcb->arg, arg, arg_length);
    // save the process name into the PCB
    memcpy(new_pcb->process_name, filename, FILENAME_LEN);
    // update the scheduler array and terminal array
    for(i = 0; i < SCHED_TOTAL; i++){
        if(SCHED_TASKS[i] == -2 || SCHED_TASKS[i] == new_pcb->parent_pid){
            SCHED_TASKS[i] = new_pcb->pid;
            break;
        }
    }
    // printf("now, the pid is %d\n",SCHED_TASKS[cur_index]);

    // printf("argument is:");
    // i = 0;
    // while (new_pcb->arg[i] != 0){
    //     printf("%c",new_pcb->arg[i]);
    //     i++;
    // }
    // printf("\n");

    // get the program start addr
    uint8_t start_addr_buffer[4];
    read_data(temp.inode_num, 24, start_addr_buffer, 4); //byte 24 - 27 store the addr of the first intr
    new_pcb->prog_start_addr = (((uint32_t) start_addr_buffer[3]) << 24) | (((uint32_t) start_addr_buffer[2]) << 16) | (((uint32_t) start_addr_buffer[1]) << 8) | ((uint32_t) start_addr_buffer[0]);

    //init the file descriptor
    for (i = 2; i < 8; i++){
        new_pcb->file_decs[i].flags = 0;
        new_pcb->file_decs[i].file_position = 0;
        new_pcb->file_decs[i].inode = 0;
    }
    //init stdin, stdout
    new_pcb->file_decs[0].flags = 1;
    new_pcb->file_decs[0].file_position = 0;
    new_pcb->file_decs[0].inode = 0;
    new_pcb->file_decs[0].op_table.open = terminal_open;
    new_pcb->file_decs[0].op_table.close = terminal_close;
    new_pcb->file_decs[0].op_table.read = terminal_read;
    new_pcb->file_decs[0].op_table.write = stdin_write;
    new_pcb->file_decs[1].flags = 1;
    new_pcb->file_decs[1].file_position = 0;
    new_pcb->file_decs[1].inode = 0;
    new_pcb->file_decs[1].op_table.open = terminal_open;
    new_pcb->file_decs[1].op_table.close = terminal_close;
    new_pcb->file_decs[1].op_table.read = stdout_read;
    new_pcb->file_decs[1].op_table.write = terminal_write;
    // process create good, add the pid counter
    pid_counter ++;

    /*context switch*/
    tss.ss0 = KERNEL_DS;
    tss.esp0 = BAMB - cur_pid * BAKB - sizeof(int32_t); // subtract 4 for dummy bits

    //store the esp and ebp
    asm volatile(
                "movl %%esp, %0;"
                "movl %%ebp, %1;"
                : "=r"(new_pcb->parent_esp), "=r"(new_pcb->parent_ebp)
                :
                : "memory"
            );
    sti();
    /*IRET*/
    asm volatile(
        "pushl %0;"
        "pushl %1;"
        "pushfl;"
        "pushl %2;"
        "pushl %3;"
        "iret;"
        : /*nothing*/
        : "r" (USER_DS), "r" (VISUAL_user_start + user_prog_size - 4), "r" (USER_CS), "r" (new_pcb->prog_start_addr)
        : "memory"
    );

    return 0;
}


/*
 * read
    * DESCRIPTION: read the file
    * INPUT: fd -- file descriptor
    *       buf -- the buffer we need to read
    *      nbytes -- the number of bytes we need to read
    * OUTPUT: None
    * RETURN VALUE: the number of bytes we read or -1 if fail
    * SIDE EFFECT: Move the file position
 */
int32_t read(int32_t fd, void* buf, int32_t nbytes){
    int32_t bytes_read = 0;
    uint32_t cur_pid;
    /*check validity*/
    if (fd < 0 || fd > 7 || buf == NULL || nbytes < 0){
        return -1;
    }
    // get the pid and pcb
    int32_t esp;
    asm volatile ("          \n\
                 movl %%esp, %0  \n\
            "
            :"=r"(esp)
            );
    cur_pid = (BAMB - esp) / BAKB;
    pcb_t* cur_pcb = get_pcb(cur_pid);
    
    // printf("enter read\n");
    if (cur_pcb->file_decs[fd].flags == 0){
        return -1;
    } 
    else {
        bytes_read = cur_pcb->file_decs[fd].op_table.read(fd, buf, nbytes);
        cur_pcb->file_decs[fd].file_position += bytes_read;
    }
    // printf("return %d byte when read\n",bytes_read);
    return bytes_read;
}

/*
 * write
    * DESCRIPTION: write to the file
    * INPUT: fd -- file descriptor
    *       buf -- the buffer we need to write
    *      nbytes -- the number of bytes we need to write
    * OUTPUT: None
    * RETURN VALUE: the number of bytes we write or -1 if fail
    * SIDE EFFECT: Move the file position
 */
int32_t write(int32_t fd, const void* buf, int32_t nbytes){
    /*check validity*/
    if (fd < 0 || fd > 7 || buf == NULL || nbytes < 0){
        return -1;
    }
    int32_t cur_pid;
    int32_t esp;
    asm volatile ("          \n\
                 movl %%esp, %0  \n\
            "
            :"=r"(esp)
            );
    cur_pid = (BAMB - esp) / BAKB;
    pcb_t* cur_pcb = get_pcb(cur_pid);
    // printf("enter write\n");
    if (cur_pcb->file_decs[fd].flags == 0){
        return -1;
    } else {
        int32_t bytes_write = cur_pcb->file_decs[fd].op_table.write(fd, buf, nbytes);
        // cur_pcb->file_decs[fd].file_position += bytes_write;
        return bytes_write;
    }
    return 0;
}

/*
 * open
    * DESCRIPTION: open the file
    * INPUT: filename -- the file name we need to open
    * OUTPUT: None
    * RETURN VALUE: the file descriptor or -1 if fail
    * SIDE EFFECT: Set the corresponding file structure and file operation table and file position and flags
 */
int32_t open(const uint8_t* filename){
    int32_t i;
    dentry_t dentry;
    // printf("enter open\n");
    /*check if the filename is valid*/
    if (strlen((char*) filename) == 0 || strlen((char*) filename) > FILENAME_LEN ||
            read_dentry_by_name(filename, &dentry) == -1){
                // printf("filename length is %d",strlen((char*) filename));
                // for (i = 0; i < strlen((char*) filename); i++){
                //     printf("%c",filename[i]);
                // }
                return -1;
    } 
    int32_t cur_pid;
    int32_t esp;
    asm volatile ("          \n\
                 movl %%esp, %0  \n\
            "
            :"=r"(esp)
            );
    cur_pid = (BAMB - esp) / BAKB;
    pcb_t* cur_pcb = get_pcb(cur_pid);
    /*insert the file to the fd array*/
    /*begin from 2 because 0 and 1 reserved for stdin/stdout, max 8 files*/
    for (i = 2; i < 8; i ++){
        if (cur_pcb->file_decs[i].flags == 0){
            cur_pcb->file_decs[i].flags = 1;
            cur_pcb->file_decs[i].file_position = 0;
            cur_pcb->file_decs[i].inode = dentry.inode_num;
            if (dentry.filetype == 0){ //rtc
                cur_pcb->file_decs[i].op_table.open = rtc_open;
                cur_pcb->file_decs[i].op_table.close = rtc_close;
                cur_pcb->file_decs[i].op_table.read = rtc_read;
                cur_pcb->file_decs[i].op_table.write = rtc_write;
            } else if (dentry.filetype == 1){ //directory
                cur_pcb->file_decs[i].op_table.open = directory_open;
                cur_pcb->file_decs[i].op_table.close = directory_close;
                cur_pcb->file_decs[i].op_table.read = directory_read;
                cur_pcb->file_decs[i].op_table.write = directory_write;
            } else if (dentry.filetype == 2){ //file
                cur_pcb->file_decs[i].op_table.open = file_open;
                cur_pcb->file_decs[i].op_table.close = file_close;
                cur_pcb->file_decs[i].op_table.read = file_read;
                cur_pcb->file_decs[i].op_table.write = file_write;
            }
            return i; //return the file descriptor
        }
    }

    return -1; //no available file descriptor
}

/*
 * close
    * DESCRIPTION: close the file
    * INPUT: fd -- file descriptor
    * OUTPUT: None
    * RETURN VALUE: 0 if success and -1 if fail
    * SIDE EFFECT: clear the corresponding file structure and file operation table and file position and flags
 */
int32_t close(int32_t fd){
    // int32_t i;
    if (fd < 2 || fd > 7){
        return -1;
    }
    int32_t cur_pid;
    int32_t esp;
    asm volatile ("          \n\
                 movl %%esp, %0  \n\
            "
            :"=r"(esp)
            );
    cur_pid = (BAMB - esp) / BAKB;
    pcb_t* cur_pcb = get_pcb(cur_pid);

    if (cur_pcb->file_decs[fd].flags == 0){
        return -1;
    } else {
        cur_pcb->file_decs[fd].flags = 0;
        cur_pcb->file_decs[fd].op_table.close(fd);
        return 0;
    }
    return 0;
}


/*
 * getargs
    * DESCRIPTION: get the argument
    * INPUT: buf -- the buffer we need to write
    *      nbytes -- the number of bytes we need to write
    * OUTPUT: None
    * RETURN VALUE: 0 if success and -1 if fail
    * SIDE EFFECT: modify the arg array in current pcb
 */
int32_t getargs(uint8_t* buf, int32_t nbytes){
    int32_t cur_pid;
    int32_t esp;
    asm volatile ("          \n\
                 movl %%esp, %0  \n\
            "
            :"=r"(esp)
            );
    cur_pid = (BAMB - esp) / BAKB;
    pcb_t* cur_pcb = get_pcb(cur_pid);
    if (cur_pcb->arg[0] == '\0' || buf == NULL || cur_pcb->arg_len > nbytes){
        return -1;
    }
    memcpy(buf,cur_pcb->arg,cur_pcb->arg_len + 1);

    return 0;
}

/*
 * vidmap
    * DESCRIPTION: map the video memory
    * INPUT: screen_start -- the start address of the screen
    * OUTPUT: None
    * RETURN VALUE: 0 if success and -1 if fail
    * SIDE EFFECT: modify video memory copy
 */
int32_t vidmap(uint8_t** screen_start){
    // validity check
    if (screen_start == NULL){
        return -1;
    }
    if (screen_start < (uint8_t**)VISUAL_user_start || screen_start >= (uint8_t**)VISUAL_user_start + user_prog_size){
        return -1;
    }
    // set the page table entry
    int32_t ret = vidmap_paging();
    *screen_start = (uint8_t*)ret;
    return video_mem_orig;
}

int32_t set_handler(int32_t signum, void* handler_address){
    return 0;
}

int32_t sigreturn(void){
    return 0;
}



int32_t execute_shell(const uint8_t* command){
    /*check validity*/
    if (command == NULL || strlen((char*) command) == 0){
        printf("invalid command\n");
        return -1;
    }

    // if (pid_counter >= MAX_PROCESS_NUMBER){
    //     printf("too many process!\n");
    //     return -1;
    // }
    /*local variables*/
    int32_t i, j;
    uint8_t filename[FILENAME_LEN] = {0};
    dentry_t temp;
    int8_t arg[FILENAME_LEN * 4] = {0};
    int32_t arg_length = 0;
    uint8_t check_exe[4]; /*check the four magic numbers*/
    pcb_t* new_pcb;
    // pcb_t* parent_pcb;
    uint32_t cur_pid;
    uint8_t* prog_addr = (uint8_t*) 0x8048000;
    uint32_t file_size = VISUAL_user_start + user_prog_size - 0x8048000;

    /*parse cmd*/
    for (i = 0; i < FILENAME_LEN; i++){
        if (command[i] == ' ' || command[i] == '\0'){ //first word as filename
            break;
        }
        filename[i] = command[i];
    }

    /*parse arg*/
    while (command[i] == ' '){
        i++;
    }
    // printf("argument is:");
    for (j = 0; i < strlen((char*) command); i++, j++){
        if (command[i] == ' ') arg[j] = ' '; //end of one arg
        if (command[i] == '\0') { //end of all args
            arg[j] = '\0';
            arg_length ++;
            break;
        } 
        arg[j] = command[i];
        // printf("%c",arg[j]);
        arg_length ++;
    }
    /*check if the file is executable*/
    if (read_dentry_by_name(filename, &temp) == -1){
        printf("invalid filename\n");
        return -1;
    }

    if (read_data(temp.inode_num, 0, check_exe, 4) == -1){
        printf("invalid file\n");
        return -1;
    }

    // magic number of exec
    if (check_exe[0] != 0x7f || check_exe[1] != 0x45 || check_exe[2] != 0x4c || check_exe[3] != 0x46){
        printf("file not executable\n");
        return -1;
    }
    
    /*paging*/
    for (i = 0; i < MAX_PROCESS_NUMBER; i++){
        if (pid_array[i] == 0){
            cur_pid = i;
            pid_array[i] = 1;
            break;
        }
    }
    user_prog_paging(cur_pid);
    /*load the file*/
    if (read_data(temp.inode_num, 0, prog_addr, file_size) == -1){
        return -1;
    }
    /*create new PCB*/
    new_pcb = (pcb_t*) (BAMB - (cur_pid + 1) * BAKB);
    new_pcb->pid = cur_pid;
    new_pcb->parent_pid = -1;
    new_pcb->saved_esp = 0;
    new_pcb->saved_ebp = 0;
    // save the argment into the PCB
    new_pcb->arg_len = arg_length;
    memcpy(new_pcb->arg, arg, arg_length);
    // save the process name into the PCB
    memcpy(new_pcb->process_name, filename, FILENAME_LEN);
    // update the scheduler array
    for(i = 0; i < SCHED_TOTAL; i++){
        if(SCHED_TASKS[i] == -2 || SCHED_TASKS[i] == new_pcb->parent_pid){
            SCHED_TASKS[i] = new_pcb->pid;
            break;
        }
    }
    // printf("now, the pid is %d\n",SCHED_TASKS[cur_index]);
    // get the program start addr
    uint8_t start_addr_buffer[4];
    read_data(temp.inode_num, 24, start_addr_buffer, 4); //byte 24 - 27 store the addr of the first intr
    new_pcb->prog_start_addr = (((uint32_t) start_addr_buffer[3]) << 24) | (((uint32_t) start_addr_buffer[2]) << 16) | (((uint32_t) start_addr_buffer[1]) << 8) | ((uint32_t) start_addr_buffer[0]);

    //init the file descriptor
    for (i = 2; i < 8; i++){
        new_pcb->file_decs[i].flags = 0;
        new_pcb->file_decs[i].file_position = 0;
        new_pcb->file_decs[i].inode = 0;
    }
    //init stdin, stdout
    new_pcb->file_decs[0].flags = 1;
    new_pcb->file_decs[0].file_position = 0;
    new_pcb->file_decs[0].inode = 0;
    new_pcb->file_decs[0].op_table.open = terminal_open;
    new_pcb->file_decs[0].op_table.close = terminal_close;
    new_pcb->file_decs[0].op_table.read = terminal_read;
    new_pcb->file_decs[0].op_table.write = stdin_write;
    new_pcb->file_decs[1].flags = 1;
    new_pcb->file_decs[1].file_position = 0;
    new_pcb->file_decs[1].inode = 0;
    new_pcb->file_decs[1].op_table.open = terminal_open;
    new_pcb->file_decs[1].op_table.close = terminal_close;
    new_pcb->file_decs[1].op_table.read = stdout_read;
    new_pcb->file_decs[1].op_table.write = terminal_write;
    // process create good, add the pid counter
    pid_counter ++;

    /*context switch*/
    tss.ss0 = KERNEL_DS;
    tss.esp0 = BAMB - cur_pid * BAKB - sizeof(int32_t); // subtract 4 for dummy bits

    //store the esp and ebp
    asm volatile(
                "movl %%esp, %0;"
                "movl %%ebp, %1;"
                : "=r"(new_pcb->saved_esp), "=r"(new_pcb->saved_ebp)
                :
                : "memory"
            );
    sti();
    /*IRET*/
    asm volatile(
        "pushl %0;"
        "pushl %1;"
        "pushfl;"
        "pushl %2;"
        "pushl %3;"
        "iret;"
        : /*nothing*/
        : "r" (USER_DS), "r" (VISUAL_user_start + user_prog_size - 4), "r" (USER_CS), "r" (new_pcb->prog_start_addr)
        : "memory"
    );

    return 0;
}

/*
 * ps
    * DESCRIPTION: print the process information
    * INPUT: None
    * OUTPUT: None
    * RETURN VALUE: 0 if success and -1 if fail
    * SIDE EFFECT: None
 */
int32_t ps(void)
{   
    printf("process information:\n");
    int i, j;
    int total_terminals = 3;
    // int total_other_process = 3;
    for (i = 0; i < total_terminals; i++){
        printf("terminal %d \n", i + 1);
        // printf("process name == shell; process id == %d\n", i);
        for (j = 0; j < MAX_PROCESS_NUMBER; j ++){
            if (pid_array[j] == 1){
                if (ancestor_pid(j) == i){
                    pcb_t* cur_pcb = get_pcb(j);
                    printf("process name == %s; process id == %d\n", cur_pcb->process_name, cur_pcb->pid);
                }
            }
        }
    }

    return 0;
}

/*
 * ancestor_pid
    * DESCRIPTION: get the ancestor pid
    * INPUT: pid -- the pid we need to check
    * OUTPUT: None
    * RETURN VALUE: the ancestor pid
    * SIDE EFFECT: None
 */
uint32_t ancestor_pid(uint32_t pid){
    uint32_t ret_pid;
    pcb_t* cur_pcb = get_pcb(pid);
    ret_pid = pid;
    while (cur_pcb->parent_pid != -1){
        ret_pid = cur_pcb->parent_pid;
        cur_pcb = get_pcb(ret_pid);
    }
    return ret_pid;
}

int32_t color(uint8_t text_color, uint8_t background_color) {   
    // printf("enter color\n");
    // printf("text_color is %d, background_color is %d\n",text_color,background_color);
    ATTRIB = (background_color << 4) | (text_color & 0x0F);
    clear();
    printf("color changed\n");
    return 0;
}
