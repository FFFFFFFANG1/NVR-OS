#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"


int main(){
    // int32_t cnt;
    uint8_t buf[4];
    uint8_t text_color;
    uint8_t background_color;
    // ece391_fdputs(1, (uint8_t*)"Change text color to: ");
    if (0 != ece391_getargs(buf, 5)) {
        ece391_fdputs(1, (uint8_t*)"Can't read color from keyboard.\n");
        return 2;
    }
    if (buf[0] != '0' || buf[1] != 'x'){
        ece391_fdputs(1, (uint8_t*)"Invalid color.\n");
        return 2;
    }
    if (buf[3] >= '0' && buf[3] <= '9'){
        text_color = buf[3] - '0';
    } else if (buf[3] >= 'A' && buf[2] <= 'F'){
        text_color = buf[3] - 'A' + 10;
    } else {
        ece391_fdputs(1, (uint8_t*)"Invalid color.\n");
        return 2;
    }
    if (buf[2] >= '0' && buf[2] <= '9'){
        background_color = buf[2] - '0';
    } else if (buf[2] >= 'A' && buf[2] <= 'F'){
        background_color = buf[2] - 'A' + 10;
    }else{
        ece391_fdputs(1, (uint8_t*)"Invalid color.\n");
        return 2;
    }
    ece391_color(text_color, background_color);
    ece391_fdputs(1, (uint8_t*)"Color changed successfully.\n");
    return 0;
}
