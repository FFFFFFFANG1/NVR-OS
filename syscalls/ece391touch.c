#include <stdint.h>
#include "ece391support.h"
#include "ece391syscall.h"

#define BUFSIZE 128

int main(){
    int32_t cnt;
    int32_t fd;     // file descriptor
    int32_t i, j;
    uint8_t buf[BUFSIZE];
    // read filename
    if (0 != ece391_getargs (buf, BUFSIZE)) {
        ece391_fdputs (1, (uint8_t*)"could not read argument\n");
        return 3;
    }
    if ((fd = ece391_open(buf)) == -1){
        /* create the new file */ 
        ece391_create(buf);
        fd = ece391_open(buf);
    }
    ece391_fdputs (1, (uint8_t*)"add you content: ");
    if (-1 == (cnt = ece391_read (0, buf, BUFSIZE-1))) {
        ece391_fdputs (1, (uint8_t*)"Can't read name from keyboard.\n");
        return 3;
    }
    if (-1 == ece391_write(fd, (void*)buf, ece391_strlen(buf))){
        ece391_fdputs(1, (uint8_t*)"Can not write into this file.\n");
        return 2;
    }
    return 0;
}
