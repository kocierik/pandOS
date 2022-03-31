#include "headers/exceptionHandler.h"

void exceptionHandler() {
    switch (CAUSE_GET_EXCCODE(getCAUSE())){
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
        case 12:
            //ProgramTraps
            break;
        case 8:
            syscall_handler();
            break;

        default: //Case 4-7 9-12
            trapHandler();
            break;
    }
}