#include "headers/exceptionHandler.h"
#include "klog.c"


void exception_handler() {
    state_t *exceptionState = (state_t *)BIOSDATAPAGE;

    switch(CAUSE_GET_EXCCODE(getCAUSE())){
        case IOINTERRUPTS:                      // Interrupt
            interrupt_handler(exceptionState);
            break;
        case 1 ... 3:                           // TLB Exception 
            //klog_print("\n\n passo dal TLB");
            tlb_handler(exceptionState);
        case 4 ... 7:                           // Trap 
        case 9 ... 12:
            //klog_print("\n\nTrapHandlerExc");
            trap_handler(exceptionState);
            break;
        case 8:                                 // System Call
            //klog_print("\n\nSyscallExc");
            syscall_handler(exceptionState);
            break;
        default:
            klog_print("cause ignota");
            break;
    }
}

/*
Nota bene: generic device, timer, e terminale, non sembrano essere mai chiamati :/
*/
void interrupt_handler(state_t *excState) {
    //klog_print("\n\ninterrupt_handler: chiamato");
    int cause = getCAUSE(); // Ritorna il registro CAUSE (3.3 pops)
    //int deviceNumber;
    if (CAUSE_IP_GET(cause, IL_IPI)) {   
        klog_print("interrupt_handler:Il_IPI");
        // Ignora
    } else if  CAUSE_IP_GET(cause, IL_CPUTIMER)    pltTimerHandler(excState);
        
      else if  CAUSE_IP_GET(cause, IL_TIMER)       intervallTimerHandler(excState);
        
      else if  CAUSE_IP_GET(cause, IL_DISK)        deviceIntHandler(IL_DISK, excState);
      else if  CAUSE_IP_GET(cause, IL_FLASH)       deviceIntHandler(IL_FLASH, excState);
      else if  CAUSE_IP_GET(cause, IL_ETHERNET)    deviceIntHandler(IL_ETHERNET, excState);
      else if  CAUSE_IP_GET(cause, IL_PRINTER)     deviceIntHandler(IL_PRINTER, excState);

      else if  CAUSE_IP_GET(cause, IL_TERMINAL)    terminalHandler();

    klog_print("\n\ninterrupt_handler:EERRORE GRAVE");
    /* TODO: ma l'acknowledgment dell'interrupt??? Scritto in slide 41 phase2_ita*/
}


void tlb_handler(state_t *excState) {
    passOrDie(PGFAULTEXCEPT, excState);
}


void trap_handler(state_t *excState) {
    passOrDie(GENERALEXCEPT, excState);
}