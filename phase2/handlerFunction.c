#include "headers/exceptionHandler.h"
extern void klog_print(char *s);
/*
* La funzione chiama l'opportuno interrupt in base al primo device che trova in funzione.
* Per vedere se un device è in funzione utilizziamo la macro CAUSE_IP_GET che legge gli opportuni bit di CAUSE e
* restituisce 1 quando un dispositivo è attivo. 
* //N.B. La funzione CAUSE_GET_IP è ben commentata dov'è definita.
*/

int interruptHandler(){
    int cause = getCAUSE(); // Ritorna il registro CAUSE (3.3 pops)
    if (CAUSE_IP_GET(cause, IL_IPI)) {   
        klog_print(">> INTERRUPT: IL_IPI\n"); 
    } else if (CAUSE_IP_GET(cause, IL_CPUTIMER)) {
        klog_print("Interrupt local timer\n");
    } else if (CAUSE_IP_GET(cause, IL_TIMER)) {
        klog_print("interrupt timer\n");
    } else if (CAUSE_IP_GET(cause, IL_DISK) || CAUSE_IP_GET(cause, IL_FLASH) ||
               CAUSE_IP_GET(cause, IL_ETHERNET) ||
               CAUSE_IP_GET(cause, IL_PRINTER)) {
        klog_print("<< INTERRUPT(GENERIC)\n");


    } else if (CAUSE_IP_GET(cause, IL_TERMINAL)) {
        klog_print("interrupt terminal\n");
    }
    return -1;
}


void passOrDie(int pageFault){
    klog_print("passOrDie chiamato, è il momento di implementarlo\n");
}

int TLBHandler(){
    passOrDie(PGFAULTEXCEPT);
    return -1;
}

void trapHandler(){
    passOrDie(GENERALEXCEPT);
}

void syscall_handler(state_t *callerProc){
//codice della syscall da prendere dal campo a0 dello stato del processo che ha alzato l'eccezione
    int syscode = (*callerProc).reg_a0;
    void * a1 = (void *) (*callerProc).reg_a1;
    void * a2 = (void *) (*callerProc).reg_a2;
    switch(syscode) {
        case CREATEPROCESS:
            createProcess(callerProc);
            break;
        case TERMPROCESS:
            terminateProcess((int *)a1);
            break;
        case PASSEREN:
            passeren((int*)a1);
            break;
        case VERHOGEN:
            verhogen((int*)a1);
            break;
        case DOIO:
            doIOdevice((int*)a1, (int)a2);
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
            getIDprocess((int)a1);
            break;
        case YIELD:
            getIDprocess((int)a1);
            break;
        default:
            break;
    }
}