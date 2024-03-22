#include <stdint.h>
#include "ece391support.h"
#include "ece391syscall.h"

#define BUFSIZE 128

int main(){
    int i, j;
    uint8_t args[BUFSIZE];
    uint8_t src_filename[BUFSIZE];
    uint8_t dsc_filename[BUFSIZE];
    // read arg
    if (0 != ece391_getargs (args, BUFSIZE)) {
        ece391_fdputs (1, (uint8_t*)"could not read argument\n");
        return 3;
    }
    // read first filename
    for (i = 0; i < ece391_strlen((uint8_t*)args) && args[i] != '\0' && args[i] != ' ' && i < BUFSIZE; i++){
        src_filename[i] = args[i];
    }
    src_filename[i] = '\0';
    // read second filename
    for (; i < ece391_strlen((uint8_t*)args) && args[i] != '\0' && args[i] == ' '; i++){};
    for (j = 0; i < ece391_strlen((uint8_t*)args) && args[i] != '\0' && i < BUFSIZE; i++, j++) {
        dsc_filename[j] = args[i];
    }
    dsc_filename[j] = '\0';
    // copy
    if (-1 == ece391_copy(src_filename,dsc_filename)){
        ece391_fdputs (1, (uint8_t*)"could not copy the file\n");
        return 2;
    }
    return 0;
}