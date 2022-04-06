#include "headers/exceptionHandler.h"
#include "klog.c"


void exceptionHandler() {
    state_t *exceptionState = (state_t *) BIOSDATAPAGE;

    switch(CAUSE_GET_EXCCODE(getCAUSE())){
        case IOINTERRUPTS: // Interrupt
            interruptHandler();
            break;
        case 1:         // TLB Exception
        case TLBINVLDL:
        case TLBINVLDS: 
            TLBHandler();
            break;
        case SYSEXCEPTION: // Chiamata una System Call
            syscall_handler(exceptionState);
            break;
        default: // TRAP
            trapHandler();
            break;
    }
}