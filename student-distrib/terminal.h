#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "types.h"
#include "lib.h"
#include "i8259.h"
#include "keyboard.h"


#define BUFFER_SIZE   128
#define TERMINAL_NUM  3

#define HISTORY_COMMAND_NUM  10


/* this represents the current working terminal */
volatile uint8_t current_terminal_index;


typedef struct terminal_t {
    int8_t cursor_x_position;
    int8_t cursor_y_position;
    uint8_t keyboard_buffer[BUFFER_SIZE]; // save the keyboard input
    uint8_t temp_buffer[BUFFER_SIZE]; // read (for ternimal)
    uint8_t input_characters_count;
    uint8_t temp_buffer_characters_count;
    volatile int8_t read_flag;

    uint8_t history_command[HISTORY_COMMAND_NUM][BUFFER_SIZE]; /*set a new buffer to store history command, it can 
                                                              store 10 commands, with each command has 128 characters*/
    uint8_t history_command_count;

    int8_t cursor_up_down_times;
} terminal_t;

extern int32_t terminal_init();
extern int32_t terminal_open( const uint8_t* filename );
extern int32_t terminal_close( int32_t fd );
extern int32_t terminal_read( int32_t fd, void* buf, int32_t nbytes );
extern int32_t terminal_write( int32_t fd, const void* buf, int32_t nbytes );

extern terminal_t terminal;

/************ Checkpoint 5 ************ */
extern void switch_terminal( int32_t index );
extern terminal_t* get_terminal_index();
extern terminal_t terminal_array[TERMINAL_NUM];
extern volatile uint8_t current_terminal_index;
extern uint8_t* background_vidmem_buf[TERMINAL_NUM];

/**************** helper functions *****************/
extern void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
extern void disable_cursor();
extern void update_cursor(int x, int y);
extern uint16_t get_cursor_position(void);
// for stdin and stdout
extern int32_t stdin_write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t stdout_read(int32_t fd, void* buf, int32_t nbytes);

#endif
