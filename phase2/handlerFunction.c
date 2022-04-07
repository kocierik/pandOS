#include "headers/exceptionHandler.h"

extern void klog_print(char *s);
extern void scheduler();

extern void insertReadyQueue(int prio, pcb_PTR p);
extern void copyState(state_t *s, state_t *p);

extern int activeProc;
extern int blockedProc;
extern pcb_t *currentActiveProc;
extern int semIntervalTimer;
extern int semDiskDevice[8];
extern int semFlashDevice[8];
extern int semNetworkDevice[8];
extern int semPrinterDevice[8];
extern int semTerminalDeviceReading[8]; 
extern int semTerminalDeviceWriting[8];

/*
* La funzione chiama l'opportuno interrupt in base al primo device che trova in funzione.
* Per vedere se un device è in funzione utilizziamo la macro CAUSE_IP_GET che legge gli opportuni bit di CAUSE e
* restituisce 1 quando un dispositivo è attivo. 
* //N.B. La funzione CAUSE_GET_IP è ben commentata dov'è definita.
*/

int getBlockedSem(int bitAddress){
    int deviceNumber = 0; 
    unsigned int devBitMap = CDEV_BITMAP_ADDR(bitAddress);
    while(devBitMap != 0){
        ++deviceNumber;
        devBitMap = devBitMap >> 1;
    }
    return deviceNumber;
}

int interruptHandler(state_t *excState){
    int cause = getCAUSE(); // Ritorna il registro CAUSE (3.3 pops)
    int deviceNumber;

    if (CAUSE_IP_GET(cause, IL_IPI)) {   
        klog_print("\n\nInterruptHandler: INTERRUPT: IL_IPI\n");         //Nulla da fare.
    } else if (CAUSE_IP_GET(cause, IL_CPUTIMER)) {

        setTIMER(TIMESLICE);
        copyState(excState, &currentActiveProc->p_s);
        insertReadyQueue(currentActiveProc->p_prio, currentActiveProc);
        --activeProc; //faccio questo perche' quando faccio l'insert prima lo aumento a caso
        scheduler();
    } else if (CAUSE_IP_GET(cause, IL_TIMER)) {

        //Interval timer
        LDIT(100000);
        pcb_PTR p;
        while((p = removeBlocked(&semIntervalTimer)) != NULL) {
            --blockedProc;
            insertReadyQueue(p->p_prio, p);
        }
        semIntervalTimer = 0;
        LDST(excState);

        klog_print("\n\nInterruptHandler: interrupt timer\n");
    } else if (CAUSE_IP_GET(cause, IL_DISK) || CAUSE_IP_GET(cause, IL_FLASH) ||
               CAUSE_IP_GET(cause, IL_ETHERNET) ||
               CAUSE_IP_GET(cause, IL_PRINTER)) {
        klog_print("\n\nInterruptHandler: INTERRUPT(GENERIC)\n");

        for (int i = 3; i < 7; i++) {
            if(CAUSE_IP_GET(cause,i)) {
                //deviceNumber = getBlockedSem(i);
                //verhogen(deviceNumber);
            }
        }
        
        
    } else if (CAUSE_IP_GET(cause, IL_TERMINAL)) {

        deviceNumber = getBlockedSem(IL_TERMINAL);
        verhogen(&semTerminalDeviceWriting[deviceNumber]);

        /*
        TODO: dobbiamo capire qua cosa fare e vedere se è giusto il codice

        */

        klog_print("\n\nInterruptHandler: interrupt terminal");
    }
    return -1;
}


void passOrDie(int pageFault, state_t *excState){
    if (currentActiveProc != NULL) {
        if (currentActiveProc->p_supportStruct == NULL) {
            klog_print("\n\n Termino il processo corrente");
            terminateProcess(0);
            scheduler();
        } else {
            copyState(excState, &currentActiveProc->p_supportStruct->sup_exceptState[pageFault]);
            int stackPtr = currentActiveProc->p_supportStruct->sup_exceptContext[pageFault].stackPtr;
            int status   = currentActiveProc->p_supportStruct->sup_exceptContext[pageFault].status;
            int pc       = currentActiveProc->p_supportStruct->sup_exceptContext[pageFault].pc;
            klog_print("\n\n Fatto il pass up");
            LDCXT(stackPtr, status, pc);
        }
    } else {
        klog_print("currentProc e' null");
    }
}

int TLBHandler(state_t *excState) {
    passOrDie(PGFAULTEXCEPT, excState);
    return -1;
}

void trapHandler(state_t *excState) {
    passOrDie(GENERALEXCEPT, excState);
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
            blockingCall = terminateProcess((int)a1);
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
            getCpuTime(callerProcState);
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
            trapHandler(callerProcState);
            break;
    }

    // dobbiamo incrementare di una word (4 byte) slide 38 di 48
    callerProcState->pc_epc += 4;

    if(blockingCall) {
        copyState(callerProcState, &currentActiveProc->p_s);
        scheduler();
    } else {
        LDST(callerProcState);
    }
}