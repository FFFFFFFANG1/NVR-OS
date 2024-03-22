#include "extra_tab.h"
#include "terminal.h"
#include "scheduler.h"
#include "paging.h"
#include "lib.h"

static uint8_t verylarge[MAX_STRING_SIZE] = "verylargetextwithverylongname.txt\0";
static uint8_t testprint[MAX_STRING_SIZE] = "testprint\0";
static uint8_t sigtest[MAX_STRING_SIZE] = "sigtest\0";
static uint8_t created[MAX_STRING_SIZE] = "created.txt\0";
static uint8_t counter[MAX_STRING_SIZE] = "counter\0";
static uint8_t pingpong[MAX_STRING_SIZE] = "pingpong\0";
static uint8_t syserr[MAX_STRING_SIZE] = "syserr\0";
static uint8_t cat[MAX_STRING_SIZE] = "cat\0";
static uint8_t grep[MAX_STRING_SIZE] = "grep\0";
static uint8_t shell[MAX_STRING_SIZE] = "shell\0";
static uint8_t fish[MAX_STRING_SIZE] = "fish\0";
static uint8_t ls[MAX_STRING_SIZE] = "ls\0";
static uint8_t frame[MAX_STRING_SIZE] = "frame\0";
static uint8_t frame0[MAX_STRING_SIZE] = "frame0.txt\0";
static uint8_t frame1[MAX_STRING_SIZE] = "frame1.txt\0";
static uint8_t hello[MAX_STRING_SIZE] = "hello\0";


int32_t press_tab(){
    int i,j;
    int start_index = 0;
    int space_or_not = 0;
    uint8_t char_buffer[MAX_STRING_SIZE];

    for (i=0; i<terminal_array[current_terminal_index].input_characters_count; i++){
        if (terminal_array[current_terminal_index].keyboard_buffer[i] == SPACE){
            space_or_not = 1;
            break;
        }
    }


    /* auto_complete starts from here */
    if (space_or_not == 1){
        start_index = i + 1;
    }

    if ((terminal_array[current_terminal_index].keyboard_buffer[start_index] == 'c') && (terminal_array[current_terminal_index].keyboard_buffer[start_index+1] == 'o')){
        memset(char_buffer, NULL, MAX_STRING_SIZE);
        memcpy(char_buffer, counter, MAX_STRING_SIZE);
    }

    else if ((terminal_array[current_terminal_index].keyboard_buffer[start_index] == 'c') && (terminal_array[current_terminal_index].keyboard_buffer[start_index+1] == 'a')){
        memset(char_buffer, NULL, MAX_STRING_SIZE);
        memcpy(char_buffer, cat, MAX_STRING_SIZE);
    }

    else if ((terminal_array[current_terminal_index].keyboard_buffer[start_index] == 'c') && (terminal_array[current_terminal_index].keyboard_buffer[start_index+1] == 'r')){
        memset(char_buffer, NULL, MAX_STRING_SIZE);
        memcpy(char_buffer, created, MAX_STRING_SIZE);
    }

    else if ((terminal_array[current_terminal_index].keyboard_buffer[start_index] == 's') && (terminal_array[current_terminal_index].keyboard_buffer[start_index+1] == 'i')){
        memset(char_buffer, NULL, MAX_STRING_SIZE);
        memcpy(char_buffer, sigtest, MAX_STRING_SIZE);
    }

    else if ((terminal_array[current_terminal_index].keyboard_buffer[start_index] == 's') && (terminal_array[current_terminal_index].keyboard_buffer[start_index+1] == 'y')){
        memset(char_buffer, NULL, MAX_STRING_SIZE);
        memcpy(char_buffer, syserr, MAX_STRING_SIZE);
    }

    else if ((terminal_array[current_terminal_index].keyboard_buffer[start_index] == 's') && (terminal_array[current_terminal_index].keyboard_buffer[start_index+1] == 'h')){
        memset(char_buffer, NULL, MAX_STRING_SIZE);
        memcpy(char_buffer, shell, MAX_STRING_SIZE);
    }

    else if ((terminal_array[current_terminal_index].keyboard_buffer[start_index] == 'f') && (terminal_array[current_terminal_index].keyboard_buffer[start_index+1] == 'i')){
        memset(char_buffer, NULL, MAX_STRING_SIZE);
        memcpy(char_buffer, fish, MAX_STRING_SIZE);
    }

    else if ((terminal_array[current_terminal_index].keyboard_buffer[start_index] == 'f') && (terminal_array[current_terminal_index].keyboard_buffer[start_index+1] == 'r') && (terminal_array[current_terminal_index].keyboard_buffer[start_index+5] == '0')){
        memset(char_buffer, NULL, MAX_STRING_SIZE);
        memcpy(char_buffer, frame0, MAX_STRING_SIZE);
    }

    else if ((terminal_array[current_terminal_index].keyboard_buffer[start_index] == 'f') && (terminal_array[current_terminal_index].keyboard_buffer[start_index+1] == 'r') && (terminal_array[current_terminal_index].keyboard_buffer[start_index+5] == '1')){
        memset(char_buffer, NULL, MAX_STRING_SIZE);
        memcpy(char_buffer, frame1, MAX_STRING_SIZE);
    }

    else if ((terminal_array[current_terminal_index].keyboard_buffer[start_index] == 'f') && (terminal_array[current_terminal_index].keyboard_buffer[start_index+1] == 'r')){
        memset(char_buffer, NULL, MAX_STRING_SIZE);
        memcpy(char_buffer, frame, MAX_STRING_SIZE);
    }

    else if ((terminal_array[current_terminal_index].keyboard_buffer[start_index] == 'g')){
        memset(char_buffer, NULL, MAX_STRING_SIZE);
        memcpy(char_buffer, grep, MAX_STRING_SIZE);
    }

    else if ((terminal_array[current_terminal_index].keyboard_buffer[start_index] == 'p')){
        memset(char_buffer, NULL, MAX_STRING_SIZE);
        memcpy(char_buffer, pingpong, MAX_STRING_SIZE);
    }

    else if ((terminal_array[current_terminal_index].keyboard_buffer[start_index] == 't')){
        memset(char_buffer, NULL, MAX_STRING_SIZE);
        memcpy(char_buffer, testprint, MAX_STRING_SIZE);
    }

    else if ((terminal_array[current_terminal_index].keyboard_buffer[start_index] == 'l')){
        memset(char_buffer, NULL, MAX_STRING_SIZE);
        memcpy(char_buffer, ls, MAX_STRING_SIZE);
    }

    else if ((terminal_array[current_terminal_index].keyboard_buffer[start_index] == 'v')){
        memset(char_buffer, NULL, MAX_STRING_SIZE);
        memcpy(char_buffer, verylarge, MAX_STRING_SIZE);
    }

    else if ((terminal_array[current_terminal_index].keyboard_buffer[start_index] == 'h')){
        memset(char_buffer, NULL, MAX_STRING_SIZE);
        memcpy(char_buffer, hello, MAX_STRING_SIZE);
    }

    else return -1;


    /* delete everything and move the cursor to the start */
    while (terminal_array[current_terminal_index].input_characters_count > 0){

		if (terminal_array[current_terminal_index].cursor_x_position == 0 && terminal_array[current_terminal_index].cursor_y_position == 0){
			return -1;
		}

		if (terminal_array[current_terminal_index].input_characters_count > 0) {
			terminal_array[current_terminal_index].input_characters_count--;
			terminal_array[current_terminal_index].temp_buffer_characters_count--;

			terminal_array[current_terminal_index].keyboard_buffer[(terminal_array[current_terminal_index].input_characters_count+1)] = '\0';
			terminal_array[current_terminal_index].temp_buffer[(terminal_array[current_terminal_index].temp_buffer_characters_count+1)] = '\0';



			multi_vidmem_mapping(current_terminal_index);

			*(uint8_t*)((char *)VIDEO_MEMORY + ((NUM_COLS * terminal_array[current_terminal_index].cursor_y_position + terminal_array[current_terminal_index].cursor_x_position-1)<<1)) = ' '; 
           	*(uint8_t*)((char *)VIDEO_MEMORY + ((NUM_COLS * terminal_array[current_terminal_index].cursor_y_position + terminal_array[current_terminal_index].cursor_x_position-1)<<1)+1) = ATTRIB;

			multi_vidmem_mapping(cur_index);
					

			terminal_array[current_terminal_index].cursor_x_position--;

	        if (terminal_array[current_terminal_index].cursor_x_position < 0) {
				terminal_array[current_terminal_index].cursor_x_position = NUM_COLS-1;
				terminal_array[current_terminal_index].cursor_y_position--;
			}


            update_cursor(terminal_array[current_terminal_index].cursor_x_position, terminal_array[current_terminal_index].cursor_y_position);
		}
    }


    /* write char_buffer to keyboard_buffer and temp_buffer*/
    for (j=0; j<MAX_STRING_SIZE; j++){
        if (char_buffer[j] == '\0') break;

        multi_vidmem_mapping(current_terminal_index);

        putc_for_keyboard(char_buffer[j]);

        multi_vidmem_mapping(cur_index);

        terminal_array[current_terminal_index].keyboard_buffer[j + start_index] = char_buffer[j];
        terminal_array[current_terminal_index].temp_buffer[j + start_index] = char_buffer[j];
    }

    terminal_array[current_terminal_index].input_characters_count = j + start_index;
    terminal_array[current_terminal_index].temp_buffer_characters_count = j + start_index;

    return 0;
}
