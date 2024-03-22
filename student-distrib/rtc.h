#ifndef _RTC_H
#define _RTC_H

#include "types.h"

//rtc inner frequency, set it to its highest 1024
#define real_frequency      1024

#define rtc_index           0x70
#define rtc_data            0x71

/* registers used to control the RTC periodic interrupt function */
/* '8' means the bit 7 is set to 1 and thus, NMI is disabled */
#define reg_a               0x8A
#define reg_b               0x8B
#define reg_c               0x8C

extern void rtc_init();//initialize retc
extern void rtc_handler();//set rtc handler
int32_t rtc_open (const uint8_t* filename);
int32_t rtc_close (int32_t fd);
int32_t rtc_read (int32_t fd, void* buf, int32_t nbytes);
int32_t rtc_write (int32_t fd, const void* buf, int32_t nbytes);

#endif
