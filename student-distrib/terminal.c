#include "terminal.h"
#include "scheduler.h"

terminal_t terminal;
terminal_t terminal_array[TERMINAL_NUM];
// this array is for memory, 2^12 = 4kB 
// Actually, in keyboard, what we write to the screen is at 0xB8000 by default
uint8_t* background_vidmem_buf[3] = {(uint8_t*)0xBA000, (uint8_t*)0xBB000, (uint8_t*)0xBC000};
volatile uint8_t current_terminal_index;

/*  Function: terminal_init()
 *  Description: initialize the terminal and cursor
 *  Input: None
 *  Output: None 
 *  Return value: None
 *  Side effect: set the cursor at the beginning space of screen
 */
int32_t terminal_init(){
    int i;
    for (i = 0; i<TERMINAL_NUM; i++){
        terminal_array[i].cursor_x_position = 0;
        terminal_array[i].cursor_y_position = 0;
        terminal_array[i].input_characters_count = 0;
        terminal_array[i].temp_buffer_characters_count = 0;
        terminal_array[i].read_flag = 0;

        memset(terminal_array[i].keyboard_buffer, NULL,  BUFFER_SIZE);
        memset(terminal_array[i].temp_buffer, NULL, BUFFER_SIZE);

        memset(terminal_array[i].history_command, NULL, HISTORY_COMMAND_NUM);
        terminal_array[i].history_command_count = 0;
        terminal_array[i].cursor_up_down_times = 0;

        enable_cursor(0, 15);
        update_cursor(terminal_array[i].cursor_x_position, terminal_array[i].cursor_y_position);
    }
    //we assume the first working terminal is the first one
    current_terminal_index = 0; 
    // terminal_idx = 0;
    background_vidmem_buf[0] = (uint8_t*)0xBA000;
    background_vidmem_buf[1] = (uint8_t*)0xBB000;
    background_vidmem_buf[2] = (uint8_t*)0xBC000;
    return 0;
}

/*  Function: terminal_open(const uint8_t* filename)
 *  Description: open the file
 *  Input: filename -- points to the file that we want to open
 *  Output: None 
 *  Return value: 0 if success, -1 if failure
 *  Side effect: None
 */
int32_t terminal_open( const uint8_t* filename ){
    if (filename == NULL) return -1;
    else return 0;
}

/*  Function: terminal_close(int32_t fd)
 *  Description: close the file
 *  Input: fd -- file descriptor
 *  Output: None 
 *  Return value: 0
 *  Side effect: set input_character_count to 0
 */
int32_t terminal_close( int32_t fd ){
    terminal_array[cur_index].input_characters_count = 0;
    return 0;
}

/*  Function: terminal_read( int32_t fd, void* buf, int32_t nbytes )
 *  Description: read the contents in keyboard buffer
 *  Input: fd -- file descriptor
           buf -- memory buffer that reads keyboard buffer
           nbytes --  number of bytes that we need to read
 *  Output: None
 *  Return value: return the bytes that we read, or -1 if failure
 *  Side effect: read from keyboard buffer to memory buffer, then clean the keyboard buffer
 */
int32_t terminal_read( int32_t fd, void* buf, int32_t nbytes ) {
    int i;
    int count = 0;
    if (buf == NULL || nbytes < 0){
        puts(" terminal_read fails ");
        return -1;
    } 
    // clean the buffer first
    memset(buf, 0, BUFFER_SIZE);  
    terminal_array[cur_index].read_flag = 0;
    // if terminal.read_flag is 0, means that we haven't pressed enter yet, so we need to wait here.
    while (terminal_array[cur_index].read_flag == 0) {};  

    for (i = 0; (i < nbytes) && (i < terminal_array[cur_index].temp_buffer_characters_count); i++){
        if (terminal_array[cur_index].temp_buffer[i] != '\0'){
            // write from temp_buffer to input buffer
            ((char*)buf)[i] = terminal_array[cur_index].temp_buffer[i]; 
            count++;
        }
    }
    //printf("temp_buffer_characters_count is %d\n", temp_buffer_characters_count);
    /* after we read, clean the two buffers in terminal struct, make sure it is ready for us to reuse again */
    memset(terminal_array[cur_index].keyboard_buffer, NULL,  BUFFER_SIZE);
    memset(terminal_array[cur_index].temp_buffer, NULL, BUFFER_SIZE);
    terminal_array[cur_index].input_characters_count = 0;
    terminal_array[cur_index].temp_buffer_characters_count = 0;
    terminal_array[cur_index].read_flag = 0;
    //printf("buf length is %d\n", count);
    // int32_t k;
    // for (k=0; k < count-1; k++){
    //     ((char*)buf)[k] = ((char*)buf)[k+1];
    // }
    // ((char*)buf)[count-1] = '\n';
    // int j;
    // for (j=0; j<i; j++){
    //     putc(((char*)buf)[j]);
    // }
    return count;
}

/*  Function: terminal_write( int32_t fd, const void* buf, int32_t nbytes )
 *  Description: write the contents in memory buffer
 *  Input: fd -- file descriptor
           buf -- memory buffer that reads keyboard buffer
           nbytes --  number of bytes that we need to read
 *  Output: None
 *  Return value: return the bytes that we read, or -1 if failure
 *  Side effect: write to screen
 */
int32_t terminal_write( int32_t fd, const void* buf, int32_t nbytes ) {
    int i;
    int count = 0;
    // printf("the first byte of buf is %c\n", *(char*)buf);
    if (buf == NULL || nbytes < 0) {
        // printf("nbyte is %d.",nbytes);
        puts(" terminal_write fails\n");
        return -1;
    }

    /* In checkpoint5, we should modify terminal_write more.
       Here, putc should write to the terminal that we scheduled, rather than current terminal.
       This is different from keyboard. */
    for (i=0; (i<nbytes); i++){
        if(((char*)buf)[i] != '\0'){
            putc_for_terminal_write(((char*)buf)[i]);  // puts everything in the buffer on screen
            count++;
        }
    }

    return count;
}

/**************** Checkpoint 5 ***************/
/*  Function: switch_terminal (int32_t index)
 *  Description: Switch to another terminal
 *  Input: index -- the index of the terminal that we are going to switch to
 *  Output: None
 *  Return value: None 
 *  Side effect: update_video_memory_paging(current_terminal)
                 Copy from video memory to background buffer of current_terminal
                 Load background buffer of target_terminal into video memory 
                 update_video_memory_paging(get_owner_terminal(current_pid))

 */
void switch_terminal(int32_t index){
    /* sanity check first */
    if ( index > 2 || index < 0 || index == current_terminal_index) {return;}
    // change the physical memory of corresponding ternimal
    multi_vidmap_update();
    /*Copy from video memory to background buffer of current_terminal*/
    memcpy((void*)background_vidmem_buf[current_terminal_index], (void*)VIDEO_MEMORY, PAGE_SIZE);
    /*Load background buffer of target_terminal into video memory */
    memcpy((void*)VIDEO_MEMORY, (void*)background_vidmem_buf[index], PAGE_SIZE);
    /*Update cursor*/
    update_cursor(terminal_array[index].cursor_x_position, terminal_array[index].cursor_y_position);
    /*update_video_memory_paging*/
    current_terminal_index = index;
    // change the 
    multi_vidmem_mapping(cur_index);
}

/*  Function: get_terminal_index()
 *  Description: get the index of current working terminal
 *  Input: None
 *  Output: None
 *  Return value: index of current terminal 
 *  Side effect: None
 */
terminal_t* get_terminal_index(){
    return &(terminal_array[current_terminal_index]);
}

/**************** helper functions *****************/
/* In text mode, the cursor does not work the same way as in high-level languages, 
 * automatically moving to one place after the last written character.
 * Instead, it is simply a blinking area that can be resized, shown, hidden, and moved by the OS.
 * These four functions are at the lowest level which communicates with hardware directly.
 * Reference comes from https://wiki.osdev.org/Text_Mode_Cursor
 */
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end) {
	outb(0x0A, 0x3D4);						
	outb((inb(0x3D5) & 0xC0) | cursor_start, 0x3D5);	
	outb(0x0B, 0x3D4);					
	outb((inb(0x3D5) & 0xE0) | cursor_end, 0x3D5);		
}

void vga_disable_cursor() {
	outb(0x0A, 0x3D4);		
	outb(0x20, 0x3D5);		
}

void update_cursor(int x, int y) {
	uint16_t pos = y * 80 + x;	
	outb(0x0F, 0x3D4);		
	outb((uint8_t) (pos & 0xFF), 0x3D5);
	outb(0x0E, 0x3D4);	
	outb((uint8_t) ((pos >> 8) & 0xFF), 0x3D5);
}

uint16_t get_cursor_position(void) {
    uint16_t pos = 0;
    outb(0x0F, 0x3D4);
    pos |= inb(0x3D5);
    outb(0x0E, 0x3D4);
    pos |= ((uint16_t)inb(0x3D5)) << 8;
    return pos;
}

// for stdin and stdout
// bad call: return -1
int32_t stdin_write(int32_t fd, const void *buf, int32_t nbytes)
{
    printf("stdin cannot write\n");
    return -1;
}

int32_t stdout_read(int32_t fd, void *buf, int32_t nbytes)
{
    printf("stdout cannot read\n");
    return -1;
}

