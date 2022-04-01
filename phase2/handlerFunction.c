#include "headers/exceptionHandler.h"

/*
* La funzione chiama l'opportuno interrupt in base al primo device che trova in funzione.
* Per vedere se un device è in funzione utilizziamo la macro CAUSE_IP_GET che legge gli opportuni bit di CAUSE e
* restituisce 1 quando un dispositivo è attivo. 
* //N.B. La funzione CAUSE_GET_IP è ben commentata dov'è definita.
*/
int interruptHandler(){
    int cause = getCAUSE(); // Ritorna il registro CAUSE (3.3 pops)
    if (CAUSE_IP_GET(cause, IL_IPI)) {   
        // pandos_kprintf(">> INTERRUPT: IL_IPI"); 
        return interrupt_ipi();
    } else if (CAUSE_IP_GET(cause, IL_CPUTIMER)) {
        return interrupt_local_timer();
    } else if (CAUSE_IP_GET(cause, IL_TIMER)) {
        return interrupt_timer();
    } else if (CAUSE_IP_GET(cause, IL_DISK) || CAUSE_IP_GET(cause, IL_FLASH) ||
               CAUSE_IP_GET(cause, IL_ETHERNET) ||
               CAUSE_IP_GET(cause, IL_PRINTER)) {
        // pandos_kprintf("<< INTERRUPT(GENERIC)\n");

        return interrupt_generic(cause);

    } else if (CAUSE_IP_GET(cause, IL_TERMINAL)) {
        return interrupt_terminal();
    }
    return -1;
}

void passOrDie(int pageFault){
    //klog_print("passOrDie chiamato, è il momento di implementarlo");
}

int TLBHandler(){
    passOrDie(PGFAULTEXCEPT);
    return -1;
}

void trapHandler(){
    passOrDie(GENERALEXCEPT);
}

void syscall_handler(int SYS){
    switch(SYS) {
        case CREATEPROCESS:
            createProcess();
            break;
        case TERMPROCESS:
            terminateProcess();
            break;
        case PASSEREN:
            passeren();
            break;
        case VERHOGEN:
            verhogen();
            break;
        case DOIO:
            doIOdevice();
            break;
        case GETTIME:
            getCpuTime();
            break;
        case CLOCKWAIT:
            waitForClock();
            break;
        case GETSUPPORTPTR:
            getSupportData();
            break;
        case GETPROCESSID:
            createProcess();
            break;
        case YIELD:
            getIDprocess();
            break;
        default:
            break;
    }
}