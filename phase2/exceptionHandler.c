#include "headers/exceptionHandler.h"
#include "klog.c"


void exception_handler() {
    state_t *exceptionState = (state_t *)BIOSDATAPAGE;

    switch(CAUSE_GET_EXCCODE(getCAUSE())){
        case IOINTERRUPTS:                      // Interrupt
            interrupt_handler(exceptionState);
            break;
        case 1 ... 3:                           // TLB Exception 
            klog_print("\n\n passo dal TLB");
            tlb_handler(exceptionState);
        case 4 ... 7:                           // Trap 
        case 9 ... 12:
            trap_handler(exceptionState);
            break;
        case 8:                                 // System Call
            syscall_handler(exceptionState);
            break;
        default:
            klog_print("cause ignota");
            break;
    }
}


void interrupt_handler(state_t *excState){
    int cause = getCAUSE(); // Ritorna il registro CAUSE (3.3 pops)
    int deviceNumber;

    if (CAUSE_IP_GET(cause, IL_IPI)) {   
        // Ignora
    } else if (CAUSE_IP_GET(cause, IL_CPUTIMER))    {
        pltTimerHandler(excState);
    } else if (CAUSE_IP_GET(cause, IL_TIMER))       {
        intervallTimerHandler(excState);
    } else if  (CAUSE_IP_GET(cause, IL_DISK)        ||
                CAUSE_IP_GET(cause, IL_FLASH)       ||
                CAUSE_IP_GET(cause, IL_ETHERNET)    ||
                CAUSE_IP_GET(cause, IL_PRINTER))    {
        deviceIntHandler(cause);
    } else if (CAUSE_IP_GET(cause, IL_TERMINAL)) {
        terminalHandler();
    }
}


void tlb_handler(state_t *excState) {
    passOrDie(PGFAULTEXCEPT, excState);
}


void trap_handler(state_t *excState) {
    passOrDie(GENERALEXCEPT, excState);
}