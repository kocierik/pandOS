#include "headers/handlerFunction.h"

extern void insert_ready_queue(int prio, pcb_PTR p);

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

// vector for Bitwise operation
int powOf2[] = {1, 2, 4, 8, 16, 32, 64, 128, 256}; 

/* INTERRUPT HANDLER FUNCTION */

// handler IL_CPUTIMER
void plt_time_handler(state_t *excState) {
    setTIMER(-2); //ACK
    copy_state(excState, &currentActiveProc->p_s);
    insert_ready_queue(currentActiveProc->p_prio, currentActiveProc);
    scheduler();
}

// handler IL_TIMER
void intervall_timer_handler(state_t *excState) {
    LDIT(100000); //ACK
    pcb_PTR p;
    while((p = removeBlocked(&semIntervalTimer)) != NULL) {
        --blockedProc;
        insert_ready_queue(p->p_prio, p);
    }
    semIntervalTimer = 0;
    load_or_scheduler(excState);
}


/* 
 * obtain the semaphore address of a generic device (NON TERMINAL)
 * Take the device's interrupt line and the device number as input (from 0 to 7)
 */
int *getDeviceSemaphore(int interruptLine, int devNumber){
    switch (interruptLine) {
        case IL_DISK:       return &semDiskDevice[devNumber];
        case IL_FLASH:      return &semFlashDevice[devNumber];
        case IL_ETHERNET:   return &semNetworkDevice[devNumber];
        case IL_PRINTER:    return &semPrinterDevice[devNumber];
    }
    return NULL;
}

// get active device with bitMap
int getDevice(int interLine) {
    unsigned int bitmap = (interLine);
    for(int i = 0; i < 8; i ++){
        if (bitmap & powOf2[i]) return i;
    }
    return -1; // ERROR
}

// handler IL_DISK | IL_FLASH | IL_ETHERNET | IL_PRINTER
void device_handler(int interLine, state_t *excState) {
    int devNumber = getDevice(interLine);
    dtpreg_t *devRegAddr = (dtpreg_t *) ( (0x10000054 + ((interLine - 3) * 0x80) + (devNumber * 0x10)));
    int *deviceSemaphore = getDeviceSemaphore(interLine, devNumber);

    unsigned int statusCode = devRegAddr->status; // save status code 
    devRegAddr->command = ACK; //Acknowledge the interrupt

    /* V-Operation */
    pcb_PTR process = removeBlocked(deviceSemaphore);
    if (process != NULL){
        process->p_s.reg_v0 = statusCode;
        --blockedProc;
        insert_ready_queue(process->p_prio, process);
    }
    load_or_scheduler(excState);
}

// handler IL_TERMINAL
void terminal_handler(state_t *excState) {
    int devNumber = getDevice(IL_TERMINAL);
    termreg_t *devRegAddr = (termreg_t *) (0x10000054 + ((IL_TERMINAL - 3) * 0x80) + (devNumber * 0x10));
    unsigned int statusCode;
    int *deviceSemaphore;

    // Check if the recv_status is ready and therefore if the reading terminal is in use
    int readingMode = (devRegAddr->recv_status == RECVD); 

    if (readingMode) {
        statusCode = devRegAddr->recv_status;
        devRegAddr->recv_command = ACK;
        deviceSemaphore = &semTerminalDeviceReading[devNumber];
    } else {
        statusCode = devRegAddr->transm_status;
        devRegAddr->transm_command = ACK;
        deviceSemaphore = &semTerminalDeviceWriting[devNumber];
    }

    pcb_PTR p = V(deviceSemaphore, NULL);
    
    if(p == NULL || p == currentActiveProc) {
        currentActiveProc->p_s.reg_v0 = statusCode;
        insert_ready_queue(currentActiveProc->p_prio, currentActiveProc);
        scheduler();
    } else {
        p->p_s.reg_v0 = statusCode;
        load_or_scheduler(excState);
    }
}

// handler TLB | TRAP
void pass_up_or_die(int pageFault, state_t *excState) {
    if (currentActiveProc != NULL) {
        if (currentActiveProc->p_supportStruct == NULL) {
            term_proc(0);
            scheduler();
        } else {
            copy_state(excState, &currentActiveProc->p_supportStruct->sup_exceptState[pageFault]);
            int stackPtr = currentActiveProc->p_supportStruct->sup_exceptContext[pageFault].stackPtr;
            int status   = currentActiveProc->p_supportStruct->sup_exceptContext[pageFault].status;
            int pc       = currentActiveProc->p_supportStruct->sup_exceptContext[pageFault].pc;
            LDCXT(stackPtr, status, pc);
        }
    }
}

// case 8 case exception_handler()
void syscall_handler(state_t *callerProcState) {
    int syscode = (*callerProcState).reg_a0;
    callerProcState->pc_epc += WORDLEN;
    
    if(((callerProcState->status << 28) >> 31)){
        callerProcState->cause |= (EXC_RI<<CAUSESHIFT);
        pass_up_or_die(GENERALEXCEPT, callerProcState);
    } else {
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
                get_cpu_time(callerProcState);
                break;
            case CLOCKWAIT:
                wait_for_clock(callerProcState);
                break;
            case GETSUPPORTPTR:
                get_support_data(callerProcState);
                break;
            case GETPROCESSID:
                get_ID_process(callerProcState);
                break;
            case YIELD:
                yield(callerProcState);
                break;
            default:
                trap_handler(callerProcState);
                break;
        }
    }
    load_or_scheduler(callerProcState);
}