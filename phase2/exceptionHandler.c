#include "headers/exceptionHandler.h"
#include "klog.c"


void exceptionHandler() {
    state_t *exceptionState = (state_t *)BIOSDATAPAGE;

    switch(CAUSE_GET_EXCCODE(getCAUSE())){
        case IOINTERRUPTS: // Interrupt // 0 
            interruptHandler();
            break;
        case 1 ... 3:         // TLB Exception 
            TLBHandler(exceptionState);
        case 4 ... 7:
        case 9 ... 12: 
            trapHandler(exceptionState);
            break;
        case 8: // Chiamata una System Call
            syscall_handler(exceptionState);
            break;
        default:
            klog_print("cause ignota");
            break;
    }
}