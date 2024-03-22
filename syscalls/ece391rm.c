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
    if (-1 == (fd = ece391_open (buf))) {
        ece391_fdputs (1, (uint8_t*)"file not found\n");
	    return 2;
    }
    if (-1 == ece391_delete(buf)){
        ece391_fdputs(1,(uint8_t*)"can't delete this file\n");
        return 3;
    }
    return 0;
}