#include "headers/exceptionHandler.h"

extern void klog_print(char *s);
extern void scheduler();

extern void insertReadyQueue(int prio, pcb_PTR p);
extern void copyState(state_t *s, pcb_PTR p);

extern pcb_t *currentActiveProc;

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

        //PLT
        klog_print("\n\nInterrupt local timer");
    } else if (CAUSE_IP_GET(cause, IL_TIMER)) {
        //Interval timer
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
    klog_print("\n\npassOrDie chiamato, e' il momento di implementarlo");
    if (currentActiveProc != NULL) {
        if (currentActiveProc->p_supportStruct == NULL) {
            klog_print("\n\n Termino il processo corrente");
            terminateProcess(0);
        } else {
            klog_print("\n\n Devo fare il pass up");
        }
    } else {
        klog_print("currentProc e' null");
    }
}

int TLBHandler() {
    passOrDie(PGFAULTEXCEPT);
    return -1;
}

void trapHandler() {
    passOrDie(GENERALEXCEPT);
}

void syscall_handler(state_t *callerProcState){
    int blockingCall = FALSE;
    
    int syscode = (*callerProcState).reg_a0;
    void * a1 = (void *) (*callerProcState).reg_a1;
    void * a2 = (void *) (*callerProcState).reg_a2;
    void * a3 = (void *) (*callerProcState).reg_a3;

    switch(syscode) {
        case CREATEPROCESS:
            (*callerProcState).reg_v0 = createProcess(a1, (int)a2, a3);
            break;
        case TERMPROCESS:
            blockingCall = terminateProcess((int *)a1);
            break;
        case PASSEREN:
            blockingCall = passeren((int*)a1);
            break;
        case VERHOGEN:
            verhogen((int*)a1);
            break;
        case DOIO:
            (*callerProcState).reg_v0 = doIOdevice((int*)a1, (int)a2);
            blockingCall = TRUE;
            break;
        case GETTIME:
            getCpuTime();
            break;
        case CLOCKWAIT:
            waitForClock();
            blockingCall = TRUE;
            break;
        case GETSUPPORTPTR:
            getSupportData();
            break;
        case GETPROCESSID:
            getIDprocess(callerProcState, (int)a1);
            break;
        case YIELD:
            yield((int)a1);
            break;
        default:
            trapHandler();
            break;
    }

    // dobbiamo incrementare di una word (4 byte) slide 38 di 48
    callerProcState->pc_epc += 4;

    if(blockingCall) {
        copyState(callerProcState, currentActiveProc);
        scheduler();
    } else {
        LDST(callerProcState);
    }
}