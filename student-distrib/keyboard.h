#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "types.h"
#include "i8259.h"
#include "lib.h"
#include "scheduler.h"




#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_REGISTER_PORT 0x64

#define VIDEO_MEMORY  0xB8000

#define NUM_COLS    80
#define NUM_ROWS    25

#define ESCAPE        0x1B
#define TAB           0x09
#define CAPS          0x3A
#define LEFT_SHIFT    0x2A
#define RIGHT_SHIFT   0x36
#define LEFT_CTRL     0x1D
#define LEFT_ALT      0x38
#define BACKSPACE     0x08
#define ENTER         0x0A


/* Scan code is used to represent whether a key is pressed or not.
 * Scan code includes make code(pressed) and break code(released).
 * Here we used the first set of scan code: original XT scan code, scan code set 1.
 * For this scan code, the break code is scan code + 0x80, which means the highest bit is set to 1.
 */
/* define our scan code */
#define ESCAPE_PRESSED         0x01
#define ESCAPE_RELEASED        0x81
#define TAB_PRESSED            0x0F
#define TAB_RELEASED           0x8F 
#define CAPS_PRESSED           0x3A
#define CAPS_RELEASED          0xBA 
#define LEFT_SHIFT_PRESSED     0x2A
#define LEFT_SHIFT_RELEASED    0xAA 
#define RIGHT_SHIFT_PRESSED    0x36
#define RIGHT_SHIFT_RELEASED   0xB6
#define LEFT_CTRL_PRESSED      0x1D
#define LEFT_CTRL_RELEASED     0x9D
#define LEFT_ALT_PRESSED       0x38
#define LEFT_ALT_RELEASED      0xB8
#define BACKSPACE_PRESSED      0x0E
#define BACKSPACE_RELEASED     0x8E
#define ENTER_PRESSED          0x1C
#define ENTER_RELEASED         0x9C



/************** Checkpoint 5 ****************/
#define F1_PRESSED             0x3B
#define F2_PRESSED             0x3C
#define F3_PRESSED             0x3D

/********************************************/



#define CURSOR_UP_PRESSED             0x48
#define CURSOR_DOWN_PRESSED           0x50



#define INPUT_CODE_NUM         59

extern void keyboard_initialize(void);

extern void keyboard_input_handler(void);

extern void backspace_pressed(void);

extern void enter_pressed(void);

extern void tab_pressed(void);





extern void putc_for_keyboard(uint8_t c);

#endif
