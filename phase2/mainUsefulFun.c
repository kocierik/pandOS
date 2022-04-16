#include "headers/mainUsefulFun.h"


/* Useful for main */

/* Initialization of global variables */
void init_global_var() {
    processId = 0;
    activeProc  = 0;
    blockedProc = 0;
    mkEmptyProcQ(&queueLowProc); //Queue of low priority process
    mkEmptyProcQ(&queueHighProc); //Queue of high priority process
    currentActiveProc = NULL;
    yieldHighProc = FALSE;
    semIntervalTimer = 0;
    for (int i = 0; i < 8; i++) {
        /* Semaphores */
        semDiskDevice[i] = 0;
        semFlashDevice[i] = 0;
        semNetworkDevice[i] = 0;
        semPrinterDevice[i] = 0;
        semTerminalDeviceReading[i] = 0;
        semTerminalDeviceWriting[i] = 0;
    }
}

/* Initialization of PassUpVector */
void init_passupvector(passupvector_t *vector) {
    vector->tlb_refill_handler = (memaddr) uTLB_RefillHandler;
    vector->tlb_refill_stackPtr = KERNELSTACK;
    vector->exception_handler = (memaddr) exception_handler;
    vector->exception_stackPtr = KERNELSTACK;
}


/* Insert process in the right queue and prioritize the process */
void insert_ready_queue(int priority, pcb_PTR process) {
    process->p_prio = priority;
    if (priority == PROCESS_PRIO_HIGH) insertProcQ(&queueHighProc, process);
    else insertProcQ(&queueLowProc, process);
}


/* Assign a unique id to a process */
void set_pid(pcb_PTR process) {
    process->p_pid = ++processId;
}



