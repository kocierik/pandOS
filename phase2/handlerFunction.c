#include "headers/handlerFunction.h"

extern void klog_print(char *s);
extern void klog_print_dec(int n);
extern void klog_print_hex(unsigned int num);
extern void scheduler();
extern void yield(state_t *s);

extern void insert_ready_queue(int prio, pcb_PTR p);
extern void copy_state(state_t *s, state_t *p);

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
extern cpu_t startTime;

int powOf2[] = {1, 2, 4, 8, 16, 32, 64, 128, 256}; //Vettore utile per l'AND tra bit.

/*
* La funzione chiama l'opportuno interrupt in base al primo device che trova in funzione.
* Per vedere se un device è in funzione utilizziamo la macro CAUSE_IP_GET che legge gli opportuni bit di CAUSE e
* restituisce 1 quando un dispositivo è attivo. 
* //N.B. La funzione CAUSE_GET_IP è ben commentata dov'è definita.
*/

void loadState(state_t *s) {
    STCK(startTime);
    LDST(s);
}


int getBlockedSem(int bitAddress) {
    int deviceNumber = 0; 
    unsigned int devBitMap = CDEV_BITMAP_ADDR(bitAddress);
    while(devBitMap != 0){
        ++deviceNumber;
        devBitMap = devBitMap >> 1;
    }
    return deviceNumber;
}


void plt_time_handler(state_t *excState) {
    klog_print("\n\nplt interrupt");
    setTIMER(-2);
    yield(excState);
}


void intervall_timer_handler(state_t *excState) {
    klog_print("\n\nIntervall Timer");
    LDIT(100000);
    pcb_PTR p;
    while((p = removeBlocked(&semIntervalTimer)) != NULL) {

        klog_print("\n\nho sbloccato qualcosa");
        --blockedProc;
        insert_ready_queue(p->p_prio, p);
        klog_print("\n\nrimesso in readyqueue");

    }

    semIntervalTimer = 0;

    if (currentActiveProc == NULL) {
        klog_print("\n\nchiamo lo schiduler");
        scheduler();
    }
    loadState(excState);
}

/*
void deviceIntHandler(int cause) {
    int deviceNumber; 
    for (int i = 3; i < 7; i++) {
        if(CAUSE_IP_GET(cause, i)) {
            deviceNumber = getBlockedSem(i); // non sono sicuro se devo utilizzare il deviceNumber o i
            devreg_t* addressReg = (devreg_t*) CAUSE_IP_GET(cause,i);
            int statusCode = addressReg->dtp.status;
            addressReg->dtp.command = ACK;
            //verhogen(deviceNumber); bisogna chiamarla??
        }
    return NULL;
}
*/

/* La funzione mi permette di ottenere l'indirizzo del semaforo di un dispositivo generico (NON TERMINALE)
 * Prendo in input l'interruptLine del dispositivo e il numero del dispositivo stesso (da 0 a 7)
 */
int *getDeviceSemaphore(int interruptLine, int devNumber){
    switch (interruptLine){
    case IL_DISK:       return &semDiskDevice[devNumber];
    case IL_FLASH:      return &semFlashDevice[devNumber];
    case IL_ETHERNET:   return &semNetworkDevice[devNumber];
    case IL_PRINTER:    return &semPrinterDevice[devNumber];
    
    default:
        klog_print("\n\ngetDeviceSemaphore: Errore Critico");
        break;
    }
    return NULL;
}

int getDevice(int interLine){
    unsigned int bitmap = (interLine);
    for(int i = 0; i < 8; i ++){
        if (bitmap & powOf2[i]) return i;
    }
    return -1; // come codice errore
}

void device_handler(int interLine, state_t *excState) {
    //memaddr *interruptLineAddr = (memaddr*) (0x10000054 + (interLine - 3)); 
    int devNumber = getDevice(interLine);
    dtpreg_t *devRegAddr = (dtpreg_t *) ( (0x10000054 + ((interLine - 3) * 0x80) + (devNumber * 0x10)));
    int *deviceSemaphore = getDeviceSemaphore(interLine, devNumber);

    unsigned int statusCode = devRegAddr->status; //Salvo lo status code 

    devRegAddr->command = ACK; //Acknowledge the interrupt

    /* Eseguo una custom V-Operation */
    pcb_PTR process = removeBlocked(deviceSemaphore);
    if (process != NULL){
        process->p_s.reg_v0 = statusCode;
        --blockedProc;
        insert_ready_queue(process->p_prio, process);
    }
    /* In caso di questo errore controlla Important Point N.2 di 3.6.1, pag 19 */
    else klog_print("\n\ndeviceIntHandler: Possibile errore");
    
    if (currentActiveProc == NULL) scheduler();
    loadState(excState);

    //Leggere Important Point 
}


void terminal_handler(state_t *excState) {
    int devNumber = getDevice(IL_TERMINAL);
    termreg_t *devRegAddr = (termreg_t *) (0x10000054 + ((IL_TERMINAL - 3) * 0x80) + (devNumber * 0x10));
    
    unsigned int statusCode;
    int *deviceSemaphore;
    int readingMode = (devRegAddr->recv_status == 5); //TODO: is it correct?

    if (readingMode && FALSE) { //TODO
        statusCode = devRegAddr->recv_status;
        devRegAddr->recv_command = ACK;
        deviceSemaphore = &semTerminalDeviceReading[devNumber];
    } else {
        statusCode = devRegAddr->transm_status;
        devRegAddr->transm_command = ACK;
        deviceSemaphore = &semTerminalDeviceWriting[devNumber];
    }

    /* Eseguo una custom V-Operation */ 
    pcb_PTR process = removeBlocked(deviceSemaphore);
    if (process != NULL){
        process->p_s.reg_v0 = statusCode;
        --blockedProc;
        insert_ready_queue(process->p_prio, process);
    }
    /* In caso di questo errore controlla Important Point N.2 di 3.6.1, pag 19 */
    else klog_print("\n\nterminalHandler: Possibile errore");

    if (currentActiveProc == NULL) scheduler();
    loadState(&currentActiveProc->p_s);
}


void pass_up_or_die(int pageFault, state_t *excState) {
    if (currentActiveProc != NULL) {
        if (currentActiveProc->p_supportStruct == NULL) {
            klog_print("\n\n Termino il processo corrente dal passup");
            terminate_process(0);
        } else {
            copy_state(excState, &currentActiveProc->p_supportStruct->sup_exceptState[pageFault]);
            int stackPtr = currentActiveProc->p_supportStruct->sup_exceptContext[pageFault].stackPtr;
            int status   = currentActiveProc->p_supportStruct->sup_exceptContext[pageFault].status;
            int pc       = currentActiveProc->p_supportStruct->sup_exceptContext[pageFault].pc;
            klog_print("\n\n Fatto il pass up");
            LDCXT(stackPtr, status, pc);
        }
    } else {
        klog_print("\n\ncurrentProc e' null");
    }
}


void syscall_handler(state_t *callerProcState) {
    
    int syscode = (*callerProcState).reg_a0;

    callerProcState->pc_epc += 4;
    
    switch(syscode) {
        case CREATEPROCESS:
            create_process(callerProcState);
            break;
        case TERMPROCESS:
            terminate_process(callerProcState);
            break;
        case PASSEREN:
            passeren(callerProcState);
            break;
        case VERHOGEN:
            verhogen(callerProcState);
            break;
        case DOIO:
            do_IO_device(callerProcState);
            break;
        case GETTIME:
            getCpuTime(callerProcState);
            break;
        case CLOCKWAIT:
            waitForClock(callerProcState);
            break;
        case GETSUPPORTPTR:
            getSupportData(callerProcState);
            break;
        case GETPROCESSID:
            getIDprocess(callerProcState);
            break;
        case YIELD:
            yield(callerProcState);
            break;
        default:
            trap_handler(callerProcState);
            break;
    }

    loadState(callerProcState);
}