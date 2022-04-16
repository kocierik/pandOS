#include "headers/syscall.h"

// SYSCALL CREATEPROCESS
void create_process(state_t *excState) {
    pcb_PTR p = allocPcb();

    state_t   *a1 = (state_t *)   (*excState).reg_a1;
    int        a2 = (int)         (*excState).reg_a2;
    support_t *a3 = (support_t *) (*excState).reg_a3;
    
    if(p == NULL)
        (*excState).reg_v0 = NOPROC;
    else {
        insertChild(currentActiveProc, p);
        copy_state(a1, &p->p_s);
        ++activeProc;
        insert_ready_queue(a2, p);
        p->p_supportStruct = a3;
        if (a3 == 0)
            p->p_supportStruct = NULL;
        (*excState).reg_v0 = p->p_pid;
    }
    load_or_scheduler(excState);
}

// SYSCALL TERMPROCESS
void terminate_process(state_t *excState) {
    int pid = (int) (*excState).reg_a1;
    term_proc(pid);
    load_or_scheduler(excState);
}


// SYSCALL PASSEREN
void passeren(state_t *excState) {
    int *semaddr = (int*) (*excState).reg_a1;
    P(semaddr, excState);
    load_or_scheduler(excState);
}

pcb_PTR P(int *semaddr, state_t *excState) { 
    pcb_PTR pid = headBlocked(semaddr);

    if((*semaddr) == 0)     block_curr_proc(excState, semaddr);
    else if(pid != NULL)    return free_process(semaddr);
    else                    --(*semaddr);

    return NULL;
}


// SYSCALL VERHOGEN
void verhogen(state_t *excState) {
    int *semaddr = (int*) (*excState).reg_a1;
    V(semaddr, excState);
    load_or_scheduler(excState);
}


pcb_PTR V(int *semaddr, state_t *excState) {
    pcb_PTR pid = headBlocked(semaddr);

    if((*semaddr) == 1)     block_curr_proc(excState, semaddr);
    else if(pid != NULL)    return free_process(semaddr);
    else                    ++(*semaddr);
    return NULL;
}

// SYCALL DOIO
void do_IO_device(state_t *excState) {
    int *cmdAddr = (int*) (*excState).reg_a1;
    int cmdValue = (int)  (*excState).reg_a2;
    
    int *devSemaphore; // Semaphore address
    devregarea_t *deviceRegs = (devregarea_t*) RAMBASEADDR;

    /* Searching which device is running looking first for terminal then for generic devices */
    for (int i = 0; i < 8; i++){
        if (& (deviceRegs->devreg[4][i].term.transm_command) == (memaddr*) cmdAddr) { //Terminal Devices Writing
            devSemaphore = &semTerminalDeviceWriting[i];
            break;
        }
        else if (& (deviceRegs->devreg[4][i].term.recv_command) == (memaddr*) cmdAddr) { //Terminal Devices Reading
            devSemaphore = &semTerminalDeviceReading[i];
            break;
        }else{
            for(int j = 0; j < 4; j++){
                if (& (deviceRegs->devreg[j][i].dtp.command) == (memaddr*) cmdAddr ){
                    if (j == 0)       devSemaphore = &semDiskDevice[i];
                    else if (j == 1)  devSemaphore = &semFlashDevice[i];  
                    else if (j == 2)  devSemaphore = &semNetworkDevice[i];
                    else              devSemaphore = &semPrinterDevice[i];
                }
                break;
            }
        }
    }

    
    *cmdAddr = cmdValue; // Execute request command
    P(devSemaphore, excState); // Call a P on the semaphore found
    load_or_scheduler(excState); // Just in case
}

// SYSCALL GETTIME
void get_cpu_time(state_t *excState) {
    update_curr_proc_time();
    excState->reg_v0 = currentActiveProc->p_time;
    load_or_scheduler(excState);
}

// SYSCALL CLOCKWAIT
void wait_for_clock(state_t *excState) {
    P(&semIntervalTimer, excState); // blocked P
    load_or_scheduler(excState); // Just in case
}

// SYSCALL GETSUPPORTPTR
void get_support_data(state_t *excState) {
    (*excState).reg_v0 = (unsigned int) ((currentActiveProc->p_supportStruct != NULL) ? currentActiveProc->p_supportStruct : NULL);
    load_or_scheduler(excState);
}

// SYSCALL GETPROCESSID
void get_ID_process(state_t *excState) {
    int parent = (int) (*excState).reg_a1;
    if (parent == 0)
        (*excState).reg_v0 = currentActiveProc->p_pid;
    else
        (*excState).reg_v0 = currentActiveProc->p_parent->p_pid;

    load_or_scheduler(excState);
}


// SYSCALL YIELD
void yield(state_t *excState) {
    copy_state(excState, &currentActiveProc->p_s);
    insert_ready_queue(currentActiveProc->p_prio, currentActiveProc);
    if(currentActiveProc->p_prio == PROCESS_PRIO_HIGH && lenQ(&queueHighProc) > 1) yieldHighProc = TRUE;
    scheduler();
}

