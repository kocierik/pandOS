#include "headers/exceptionHandler.h"

void exceptionHandler() {
    int syscode = -1; //codice della syscall da prendere dal campo a0 dello stato del current process
    switch(CAUSE_GET_EXCCODE(getCAUSE())){
        case 0: 
            interruptHandler();
            break;
        case 1:
        case 2:
        case 3:
            TLBHandler();
            break;
        case 4:
        case 5:
        case 7:
        case 9:
        case 10:
        case 11:
        case 12: //Case 4-7 9-12
            trapHandler();
            break;
        case 8:
            syscall_handler(syscode);
            break;

        default: //ERROR
            break;
    }
}