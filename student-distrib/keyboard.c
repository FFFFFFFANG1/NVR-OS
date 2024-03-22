#include "keyboard.h"
#include "terminal.h"
#include "lib.h"
#include "extra_tab.h"

uint8_t capslock  = 0;
uint8_t shift  = 0;
uint8_t ctrl = 0;
uint8_t alt = 0;



static char* video_mem = (char *)VIDEO_MEMORY;


/* The order of this table is built according to scan code set 1. 
 * There are 59 elements in this table, because number 1 starts from 0x02, so we need to add 2 null at the beginning.
 * This table includes all keys in keyboard, if there's no reponse for this key, we set it to null.
 * For more details, please check https://wiki.osdev.org/Keyboard
 * build table for input code, including CAPS and SHIFT 
 * \0: null, \b:backspace, \t:tab, \n:newline, \\:represent \ to avoid confusion
 */
char input_table[INPUT_CODE_NUM] = {
	'\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
	'\b', '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p','[', ']', '\n', '\0',
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l' ,';', '\'', '`', '\0', '\\',
	'z', 'x', 'c', 'v', 'b', 'n', 'm',',', '.', '/', '\0', '\0', '\0', ' ', '\0',
};

/* when shift is pressed */
char input_table_shift[INPUT_CODE_NUM] = {
	'\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',
	'\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P','{', '}', '\n', '\0',
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L' ,':', '\"', '~', '\0', '|',
	'Z', 'X', 'C', 'V', 'B', 'N', 'M','<', '>', '?', '\0', '\0', '\0', ' ', '\0',
};

/* when caps is pressed */
char input_table_capslock[INPUT_CODE_NUM] = {
	'\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
	'\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P','[', ']', '\n', '\0',
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L' ,';', '\'', '`', '\0', '\\',
	'Z', 'X', 'C', 'V', 'B', 'N', 'M',',', '.', '/', '\0', '\0', '\0', ' ', '\0',
};

/* when shift and caps are pressed */
char input_table_shift_capslock[INPUT_CODE_NUM] = {
	'\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',
	'\b', '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p','{', '}', '\n', '\0',
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l' ,':', '\"', '~', '\0', '|',
	'z', 'x', 'c', 'v', 'b', 'n', 'm','<', '>', '?', '\0', '\0', '\0', ' ', '\0',
};     






/*
*   Function: keyboard_initializer
*   Description: initialize the keyboard's irq, IR1 on PIC
*   Input: None
*   Output: None
*   Return value: None
*   Side effect: PIC's 1st irq will be enabled. 
*/
void
keyboard_initialize(void){
    enable_irq(1);   // because keyboard is connected to IR1 in PIC, so we enable it
    
}



/*
*   Function: keyboard_input_handler
*   Description: handle with keyboard interrupt and put input character on screen
*   Input: None
*   Output: The character that we pressed, including backspace, enter and tab
*   Return value: None
*   Side effect: put the input character on screen
*/
void 
keyboard_input_handler(void){
    cli();
	uint8_t input_code;
	uint8_t character;
	int i,j;

	int history_command_index;
	
	/******************* checkpoint 5 ******************/
	terminal_t* current_terminal;
	current_terminal = get_terminal_index();
	if (current_terminal == NULL){
		send_eoi(1);
		sti();
		return;
	}
    /***************************************************/
	/* Read from port to get the current scan code. */
	input_code = inb(KEYBOARD_DATA_PORT);
	/* check whether shift and capslock are pressed*/
	switch (input_code) {
		case LEFT_SHIFT_PRESSED:	shift = 1;		break;
		case LEFT_SHIFT_RELEASED:	shift = 0;		break;
		case RIGHT_SHIFT_PRESSED:	shift = 1;		break;
		case RIGHT_SHIFT_RELEASED:	shift = 0;		break;

		case LEFT_CTRL_PRESSED:     ctrl  = 1;      break;
		case LEFT_CTRL_RELEASED:    ctrl  = 0;      break;

		case LEFT_ALT_PRESSED:      alt   = 1;      break;
		case LEFT_ALT_RELEASED:     alt   = 0;      break;

		case CAPS_PRESSED:		    capslock = !capslock;	break;


		/************* Checkpoint 5 ***************/
		case F1_PRESSED:
			if (alt == 1) switch_terminal(0);
			break;

		case F2_PRESSED:
			if (alt == 1) switch_terminal(1);
			break;

		case F3_PRESSED:
			if (alt == 1) switch_terminal(2);
			break;
		/******************************************/
		


		/*****************  extra  ****************/
		case CURSOR_UP_PRESSED:

			for (i=0; i<128; i++){

				if (terminal_array[current_terminal_index].cursor_x_position == 0 && terminal_array[current_terminal_index].cursor_y_position == 0){
					return;
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


			terminal_array[current_terminal_index].cursor_up_down_times++;
			if (terminal_array[current_terminal_index].cursor_up_down_times > terminal_array[current_terminal_index].history_command_count){
				terminal_array[current_terminal_index].cursor_up_down_times = terminal_array[current_terminal_index].history_command_count;
			}
			history_command_index = terminal_array[current_terminal_index].history_command_count - terminal_array[current_terminal_index].cursor_up_down_times;

			if (terminal_array[current_terminal_index].history_command_count == 0) break;
			if (history_command_index < 0) break;

			for (j=0; j<MAX_STRING_SIZE; j++){
				if (terminal_array[current_terminal_index].history_command[history_command_index][j] == '\0') break;

				terminal_array[current_terminal_index].temp_buffer[j] = terminal_array[current_terminal_index].history_command[history_command_index][j];
				terminal_array[current_terminal_index].temp_buffer_characters_count++;
				terminal_array[current_terminal_index].keyboard_buffer[j] = terminal_array[current_terminal_index].history_command[history_command_index][j];
				terminal_array[current_terminal_index].input_characters_count++;

				multi_vidmem_mapping(current_terminal_index);

				putc_for_keyboard(terminal_array[current_terminal_index].temp_buffer[j]);

				multi_vidmem_mapping(cur_index);

				terminal_array[current_terminal_index].keyboard_buffer[j] = terminal_array[current_terminal_index].history_command[history_command_index][j];
				terminal_array[current_terminal_index].temp_buffer[j] = terminal_array[current_terminal_index].history_command[history_command_index][j];
			}
			
			break;
		

		case CURSOR_DOWN_PRESSED:

			for (i=0; i<128; i++){

				if (terminal_array[current_terminal_index].cursor_x_position == 0 && terminal_array[current_terminal_index].cursor_y_position == 0){
					return;
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


			terminal_array[current_terminal_index].cursor_up_down_times--;
			if (terminal_array[current_terminal_index].cursor_up_down_times < 0){
				terminal_array[current_terminal_index].cursor_up_down_times = 0;
			}
			
			history_command_index = terminal_array[current_terminal_index].history_command_count - terminal_array[current_terminal_index].cursor_up_down_times;

			if (terminal_array[current_terminal_index].history_command_count == 0) break;
			if (history_command_index < 0) break;

			for (j=0; j<MAX_STRING_SIZE; j++){
				if (terminal_array[current_terminal_index].history_command[history_command_index][j] == '\0') break;

				terminal_array[current_terminal_index].temp_buffer[j] = terminal_array[current_terminal_index].history_command[history_command_index][j];
				terminal_array[current_terminal_index].temp_buffer_characters_count++;
				terminal_array[current_terminal_index].keyboard_buffer[j] = terminal_array[current_terminal_index].history_command[history_command_index][j];
				terminal_array[current_terminal_index].input_characters_count++;

				multi_vidmem_mapping(current_terminal_index);

				putc_for_keyboard(terminal_array[current_terminal_index].temp_buffer[j]);

				multi_vidmem_mapping(cur_index);

				terminal_array[current_terminal_index].keyboard_buffer[j] = terminal_array[current_terminal_index].history_command[history_command_index][j];
				terminal_array[current_terminal_index].temp_buffer[j] = terminal_array[current_terminal_index].history_command[history_command_index][j];
			}
			
			break;

		// 	terminal_array[current_terminal_index].cursor_up_down_times--;
		// 	if (terminal_array[current_terminal_index].cursor_up_down_times < 0){
		// 		terminal_array[current_terminal_index].cursor_up_down_times = 0;
		// 	}
		// 	if (terminal_array[current_terminal_index].history_command_count == 0) break;
		// 	if (terminal_array[current_terminal_index].current_history_command_index <= 0) break;

		// 	update_cursor(8, terminal_array[current_terminal_index].cursor_y_position);

		// 	terminal_array[current_terminal_index].current_history_command_index--;
		// 	current_history_command_length = strlen((int8_t*)terminal_array[current_terminal_index].history_command[terminal_array[current_terminal_index].current_history_command_index]);

		// 	for (i=0; i<current_history_command_length; i++){
		// 		putc_for_keyboard((int)terminal_array[current_terminal_index].history_command[terminal_array[current_terminal_index].current_history_command_index][i]);
		// 	}

		// 	for (i=0; i<current_history_command_length; i++){
		// 		terminal_array[current_terminal_index].keyboard_buffer[i] = terminal_array[current_terminal_index].history_command[terminal_array[current_terminal_index].current_history_command_index][i];
		// 		terminal_array[current_terminal_index].temp_buffer[i] = terminal_array[current_terminal_index].history_command[terminal_array[current_terminal_index].current_history_command_index][i];
		// 	}

			

		/******************************************/
		default:
			if (input_code >= INPUT_CODE_NUM) break;	/* Invalid input code */
			if (shift == 1 && capslock == 1) {
				character = input_table_shift_capslock[input_code];
			} 
            if (shift == 1 && capslock == 0) {
				character = input_table_shift[input_code];
			} 
            if (capslock == 1 && shift == 0) {
				character = input_table_capslock[input_code];
			} 
            if (capslock == 0 && shift == 0) {
				character = input_table[input_code];
			}

			/* if we press ctrl+l, we should clear the screen and put the cursor back at the top of screen */
			if ((ctrl == 1 && character == 'l') || (ctrl == 1 && character == 'L')){

				multi_vidmem_mapping(current_terminal_index);

				terminal_array[current_terminal_index].cursor_x_position = 0;
				terminal_array[current_terminal_index].cursor_y_position = 0;

				multi_vidmem_mapping(current_terminal_index);
		
				terminal_array[current_terminal_index].cursor_x_position = 0;
				terminal_array[current_terminal_index].cursor_y_position = 0;
			

				clear();

				update_cursor(terminal_array[current_terminal_index].cursor_x_position, terminal_array[current_terminal_index].cursor_y_position);

				multi_vidmem_mapping(cur_index); 

				send_eoi(1);
				return;
			}



/*Notes: Starts from here, the following part is new for Checkpoint5. I still reserve the old part for Checkpoint2 here
  Here I combine the two parts for buffer and video memory. The most significant difference is mapping functions. */
			else if (character == '\t'){
				int32_t ret;
				ret = press_tab();
				//printf("finish press_tab!");
				if (ret < 0){
					tab_pressed();
				}
				
				
			}
		
			else if (character == '\n'){
				terminal_array[current_terminal_index].keyboard_buffer[terminal_array[current_terminal_index].input_characters_count] = '\n';
				terminal_array[current_terminal_index].temp_buffer[terminal_array[current_terminal_index].temp_buffer_characters_count] = '\n';

				terminal_array[current_terminal_index].input_characters_count++;
				terminal_array[current_terminal_index].temp_buffer_characters_count++;

				terminal_array[current_terminal_index].read_flag = 1;

				putc_for_keyboard('\n');


				memset(terminal_array[current_terminal_index].keyboard_buffer, NULL,  BUFFER_SIZE);
				terminal_array[current_terminal_index].input_characters_count = 0;

				terminal_array[current_terminal_index].cursor_up_down_times = 0;

				if (terminal_array[current_terminal_index].history_command_count < 10){
					for (i=0; i<terminal_array[current_terminal_index].temp_buffer_characters_count-1; i++){
						terminal_array[current_terminal_index].history_command[terminal_array[current_terminal_index].history_command_count][i] = terminal_array[current_terminal_index].temp_buffer[i];
					}
					terminal_array[current_terminal_index].history_command_count++;
				}
				else if (terminal_array[current_terminal_index].history_command_count == 10){
					for (i=1; i<10; i++){
						for (j=0; j<128; j++){
							terminal_array[current_terminal_index].history_command[i-1][j] = terminal_array[current_terminal_index].history_command[i][j];
						}
					}

					for (i=0; i<terminal_array[current_terminal_index].temp_buffer_characters_count-1; i++){
						terminal_array[current_terminal_index].history_command[9][i] = terminal_array[current_terminal_index].temp_buffer[i];
					}
					
					terminal_array[current_terminal_index].history_command_count = 10;
				}



			}


			else if (character == '\b'){
				if (terminal_array[current_terminal_index].input_characters_count == 0){
					sti();
                	send_eoi(1);
                	return; 
				}

				if (terminal_array[current_terminal_index].cursor_x_position == 0 && terminal_array[current_terminal_index].cursor_y_position == 0){
					return;
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
	
			else{
				if (terminal_array[current_terminal_index].input_characters_count < 127){
					terminal_array[current_terminal_index].keyboard_buffer[current_terminal->input_characters_count] = character;
					terminal_array[current_terminal_index].temp_buffer[current_terminal->temp_buffer_characters_count] = character;
					terminal_array[current_terminal_index].temp_buffer_characters_count++;
					terminal_array[current_terminal_index].input_characters_count++;
					putc_for_keyboard(character);
				}
				else break;;
			}
	}

	
	

	send_eoi(1);
	sti();
}

/*****************************Old Version for Checkpoint2 *******************************/


			// /* this part is used to handle with special keys and normal keys separately, then update cursor's location */
			// switch (character){
			// 	case '\n': enter_pressed(); break;
			// 	case '\b': if (terminal_array[current_terminal_index].input_characters_count > 0) {backspace_pressed();} break;
			// 	case '\t': tab_pressed(); break;
				
			// 	default:
			// 	/* check if we need to do scrolling here, or just update cursor*/
			// 		if (terminal_array[current_terminal_index].cursor_x_position >= NUM_COLS){
			// 			if (terminal_array[current_terminal_index].cursor_y_position == 24){
			// 				for(j = 1; j < NUM_ROWS; j++){
			// 					for(i = 0; i<NUM_COLS; i++){

			// 						/* copy the contents of row 1-24 and move them to row 0-23 */
			// 						*(uint8_t*)(video_mem + ((NUM_COLS * (j-1) + i)<<1)) =*(uint8_t*)(video_mem + ((NUM_COLS * j + i)<<1));
			// 						*(uint8_t*)(video_mem + ((NUM_COLS * (j-1) + i)<<1) + 1) =ATTRIB;

			// 						/* add a new line at the bottom of screen */
			// 						if(j == (NUM_ROWS - 1)){
			// 							*(uint8_t*)(video_mem + ((NUM_COLS * j + i)<<1)) = ' ';
			// 							*(uint8_t*)(video_mem + ((NUM_COLS * j + i)<<1) + 1) = ATTRIB;
			// 						}
			// 					}
			// 				}

			// 				terminal_array[current_terminal_index].cursor_y_position--;

			// 				update_cursor(0,24);
			// 			}


			// 			terminal_array[current_terminal_index].cursor_x_position = 0;
			// 			terminal_array[current_terminal_index].cursor_y_position++;
			// 			update_cursor(terminal_array[current_terminal_index].cursor_x_position, terminal_array[current_terminal_index].cursor_y_position);
			// 		}

			

			// 		if (terminal_array[current_terminal_index].input_characters_count < 127){
			// 			*(uint8_t *)(video_mem + ((NUM_COLS * terminal_array[current_terminal_index].cursor_y_position) + (terminal_array[current_terminal_index].cursor_x_position << 1))) = character;
			// 			*(uint8_t *)(video_mem + ((NUM_COLS * terminal_array[current_terminal_index].cursor_y_position) + (terminal_array[current_terminal_index].cursor_x_position << 1)) + 1) = ATTRIB;
			// 			terminal_array[current_terminal_index].cursor_x_position++;
			// 			break;
			// 		}

			// 		if (terminal_array[current_terminal_index].input_characters_count >= 128) break;


			// }
			// update_cursor(terminal_array[current_terminal_index].cursor_x_position, terminal_array[current_terminal_index].cursor_y_position);





			/* the following part is associated to buffer, after we press any key, we should write it into the keyboard*/

			/* If we pressed enter, add '\n' to the last of keyboard buffer, increment input_characters_count and change the read flag.
			   We change the read flag because enter also has the effect that make the memory buffer read from keyboard buffer.
			   This flag is 0 initially, each time if we press enter, set it to 1 so memory buffer can read data from keyboard buffer */
	// 		if (character == '\n' || character ==  '\r'){
	// 			putc_for_keyboard(character);

	// 			terminal_array[current_terminal_index].input_characters_count++;
	// 			terminal_array[current_terminal_index].temp_buffer_characters_count++;

	// 			terminal_array[current_terminal_index].keyboard_buffer[terminal_array[current_terminal_index].input_characters_count] = '\n';
	// 			terminal_array[current_terminal_index].temp_buffer[terminal_array[current_terminal_index].temp_buffer_characters_count] = '\n';
	// 			terminal_array[current_terminal_index].read_flag = 1;
	// 		}
	

	// 		/* If we pressed backspace, decrement input_character_count and add a '\0' at the end of keyboard buffer */
	// 		if (character == '\b'){
	// 			if (terminal_array[current_terminal_index].input_characters_count > 0) {
	// 				putc_for_keyboard(character);

	// 				terminal_array[current_terminal_index].input_characters_count--;
	// 				terminal_array[current_terminal_index].temp_buffer_characters_count--;

	// 				terminal_array[current_terminal_index].keyboard_buffer[(terminal_array[current_terminal_index].input_characters_count+1)] = '\0';
	// 				terminal_array[current_terminal_index].temp_buffer[(terminal_array[current_terminal_index].temp_buffer_characters_count+1)] = '\0';
	// 			}
				
	// 		}


	// 		if (character == '\t'){
	// 			putc_for_keyboard(character);
	// 		}


	// 		/* Other cases, we just modify keyboard_buffer and input_character_count */
	// 		else if (character != '\0'){
	// 			if (terminal_array[current_terminal_index].input_characters_count < 128){
	// 				putc_for_keyboard(character);

	// 				terminal_array[current_terminal_index].temp_buffer_characters_count++;
	// 				terminal_array[current_terminal_index].input_characters_count++;

	// 				terminal_array[current_terminal_index].keyboard_buffer[current_terminal->input_characters_count] = character;
	// 				terminal_array[current_terminal_index].temp_buffer[current_terminal->temp_buffer_characters_count] = character;
	// 			}
	// 			else break;
	// 		}



/* void putc_for_keyboard(uint8_t c);
 * Inputs: uint_8* c = character to print
 * Return Value: void
 * Function: Output a character to the console */
void putc_for_keyboard(uint8_t c) {
    int i,j;
	int flags;
	cli_and_save(flags);
    multi_vidmem_mapping(current_terminal_index);
		if(c == '\n' || c == '\r') {
			enter_pressed();
		} 
		else {
			*(uint8_t *)(video_mem + ((NUM_COLS * terminal_array[current_terminal_index].cursor_y_position + terminal_array[current_terminal_index].cursor_x_position) << 1)) = c;
			*(uint8_t *)(video_mem + ((NUM_COLS * terminal_array[current_terminal_index].cursor_y_position + terminal_array[current_terminal_index].cursor_x_position) << 1) + 1) = ATTRIB;
			terminal_array[current_terminal_index].cursor_x_position++;

			if (terminal_array[current_terminal_index].cursor_x_position >= NUM_COLS){
				terminal_array[current_terminal_index].cursor_x_position = terminal_array[current_terminal_index].cursor_x_position % NUM_COLS;
				terminal_array[current_terminal_index].cursor_y_position++;

				if(terminal_array[current_terminal_index].cursor_y_position>= NUM_ROWS){ // the number if rows, NUM_ROWS
						for(j = 1; j < NUM_ROWS; j++){
							for(i = 0; i<NUM_COLS; i++){

								/* copy the contents of row 1-24 and move them to row 0-23 */
								*(uint8_t*)(video_mem + ((NUM_COLS * (j-1) + i)<<1)) =*(uint8_t*)(video_mem + ((NUM_COLS * j + i)<<1));
								*(uint8_t*)(video_mem + ((NUM_COLS * (j-1) + i)<<1) + 1) =ATTRIB;

								/* add a new line at the bottom of screen */
								if(j == (NUM_ROWS - 1)){
									*(uint8_t*)(video_mem + ((NUM_COLS * j + i)<<1)) = ' ';
									*(uint8_t*)(video_mem + ((NUM_COLS * j + i)<<1) + 1) = ATTRIB;
								}
							}
						}

						terminal_array[current_terminal_index].cursor_y_position--;

						update_cursor(0,24);

				}		

				terminal_array[current_terminal_index].cursor_y_position = terminal_array[current_terminal_index].cursor_y_position % NUM_ROWS;
			}
		}

	update_cursor(terminal_array[current_terminal_index].cursor_x_position, terminal_array[current_terminal_index].cursor_y_position);


    multi_vidmem_mapping(cur_index);


	restore_flags(flags);
}






/*  Function: enter_pressed()
 *  Description: called whenever we need to add a new line, it also needs to handle the case when scrolling the whole page
 *  Input: None
 *  Output: None
 *  Return value: None
 *  Side effect: add a new line, or scroll the whole page then add a new line
 */
void enter_pressed(void){
	int i,j;
	terminal_array[current_terminal_index].cursor_x_position = 0;
	terminal_array[current_terminal_index].cursor_y_position++;
	

	/* if y_position is going out of the screen, we should move the whole page down, move the cursor up by one line */
	if(terminal_array[current_terminal_index].cursor_y_position>= NUM_ROWS){ // the number if rows, NUM_ROWS
        for(j = 1; j < NUM_ROWS; j++){
			for(i = 0; i<NUM_COLS; i++){

				/* copy the contents of row 1-24 and move them to row 0-23 */
				*(uint8_t*)(video_mem + ((NUM_COLS * (j-1) + i)<<1)) =*(uint8_t*)(video_mem + ((NUM_COLS * j + i)<<1));
				*(uint8_t*)(video_mem + ((NUM_COLS * (j-1) + i)<<1) + 1) = ATTRIB;

				/* add a new line at the bottom of screen */
				if(j == (NUM_ROWS - 1)){
					*(uint8_t*)(video_mem + ((NUM_COLS * j + i)<<1)) = ' ';
					*(uint8_t*)(video_mem + ((NUM_COLS * j + i)<<1) + 1) = ATTRIB;
				}
			}
		}

		terminal_array[current_terminal_index].cursor_y_position--;

		update_cursor(0,24);


    }

	memset(terminal_array[current_terminal_index].keyboard_buffer, NULL,  BUFFER_SIZE);
	terminal_array[current_terminal_index].input_characters_count = 0;
}


/*  Function: backspace_pressed()
 *  Description: called when we press backspace, it should go back
 *  Input: None
 *  Output: None
 *  Return value: None
 *  Side effect: deletes any character, and move the cursor back by a character
 * 
 */
void backspace_pressed(void){
	/* if we press backspace at the beginning, nothing happens*/
	if (terminal_array[current_terminal_index].cursor_x_position == 0 && terminal_array[current_terminal_index].cursor_y_position == 0) return;

	terminal_array[current_terminal_index].cursor_x_position--;

	if (terminal_array[current_terminal_index].cursor_x_position < 0) {
		terminal_array[current_terminal_index].cursor_x_position = 0; // not allow the cursor to become negative number
		/* if we press words more than a row, we should go back to last row*/
		if (terminal_array[current_terminal_index].input_characters_count > NUM_COLS-1){
			terminal_array[current_terminal_index].cursor_x_position = NUM_COLS-1;
			terminal_array[current_terminal_index].cursor_y_position--;
		}
	}
	*(uint8_t *)(video_mem + ((NUM_COLS * terminal_array[current_terminal_index].cursor_y_position + terminal_array[current_terminal_index].cursor_x_position) << 1)) = ' ';

	update_cursor(terminal_array[current_terminal_index].cursor_x_position, terminal_array[current_terminal_index].cursor_y_position);
}



/*  Function: tab_pressed()
 *  Description: called when we press tab
 *  Input: None
 *  Output: None
 *  Return value: None
 *  Side effect: add 4 empty spaces
 */
void tab_pressed(void){
	int i,j;
	
	if (terminal_array[current_terminal_index].input_characters_count < BUFFER_SIZE-1){
		for (i=0; i<4; i++){  // tab has 4 empty spaces
			terminal_array[current_terminal_index].cursor_x_position++;

			if (terminal_array[current_terminal_index].cursor_x_position >= NUM_COLS) {
				terminal_array[current_terminal_index].cursor_x_position = terminal_array[current_terminal_index].cursor_x_position % NUM_COLS;
				terminal_array[current_terminal_index].cursor_y_position++;		

				if(terminal_array[current_terminal_index].cursor_y_position>= NUM_ROWS){ // the number if rows, NUM_ROWS
					for(j = 1; j < NUM_ROWS; j++){
						for(i = 0; i<NUM_COLS; i++){

							/* copy the contents of row 1-24 and move them to row 0-23 */
							*(uint8_t*)(video_mem + ((NUM_COLS * (j-1) + i)<<1)) =*(uint8_t*)(video_mem + ((NUM_COLS * j + i)<<1));
							*(uint8_t*)(video_mem + ((NUM_COLS * (j-1) + i)<<1) + 1) =ATTRIB;

							/* add a new line at the bottom of screen */
							if(j == (NUM_ROWS - 1)){
								*(uint8_t*)(video_mem + ((NUM_COLS * j + i)<<1)) = ' ';
								*(uint8_t*)(video_mem + ((NUM_COLS * j + i)<<1) + 1) = ATTRIB;
							}
						}
					}

					terminal_array[current_terminal_index].cursor_y_position--;

					update_cursor(0,24);


				}		

			}

			/* add 4 empry spaces */
			*(uint8_t *)(video_mem + ((NUM_COLS * terminal_array[current_terminal_index].cursor_y_position + terminal_array[current_terminal_index].cursor_x_position) << 1)) = ' ';
			*(uint8_t *)(video_mem + ((NUM_COLS * terminal_array[current_terminal_index].cursor_y_position + terminal_array[current_terminal_index].cursor_x_position) << 1) + 1) = ATTRIB;
			
			terminal_array[current_terminal_index].keyboard_buffer[terminal_array[current_terminal_index].input_characters_count++] = ' ';
			terminal_array[current_terminal_index].temp_buffer[terminal_array[current_terminal_index].temp_buffer_characters_count++] = ' ';
			
		}

		update_cursor(terminal_array[current_terminal_index].cursor_x_position, terminal_array[current_terminal_index].cursor_y_position);

	}
}

















