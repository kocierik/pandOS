#include "headers/syscall.h"

/* Exsternal variable */
extern int activeProc;
extern int blockedProc;
extern struct list_head queueLowProc;
extern struct list_head queueHighProc;
extern pcb_t *currentActiveProc;
extern int yieldHighProc;
extern int semIntervalTimer;
extern int semDiskDevice[8];
extern int semFlashDevice[8];
extern int semNetworkDevice[8];
extern int semPrinterDevice[8];
extern int semTerminalDeviceReading[8]; 
extern int semTerminalDeviceWriting[8];
extern cpu_t startTime;

/* Exsternal function */
extern void insert_ready_queue(int prio, pcb_PTR p);


/* HELPER FUNCTION */

void copy_state(state_t *new, state_t *old) {
    old->cause = new->cause;
    old->entry_hi = new->entry_hi;
    for (int i = 0; i < STATE_GPR_LEN; i++)
        old->gpr[i] = new->gpr[i];
    old->hi = new->hi;
    old->lo = new->lo;
    old->pc_epc = new->pc_epc;
    old->status = new->status;
}


// remove child process
void term_proc_and_child(pcb_PTR parent) {
    pcb_PTR p;
    while(!isPcbFree(parent->p_pid)) {
        p = parent;
        while(!emptyChild(p))
            p = container_of(p->p_child.next, pcb_t, p_sib);
        
        term_single_proc(p); // termino p
    }
}


void term_single_proc(pcb_PTR p) {
    --activeProc;
    if (p->p_semAdd != NULL) {
        --blockedProc;
        outBlocked(p);
    }    
    if (p == currentActiveProc)
        currentActiveProc = NULL;
    
    outChild(p); // take away p as son so it goes on
    freePcb(p);
}

// find the blocking pcb in the two queues
pcb_PTR find_pcb(int pid) {
    struct list_head *pos;
    pcb_PTR p;

    list_for_each(pos, &queueHighProc) {
        if((p = container_of(pos, pcb_t, p_list))->p_pid == pid)
            return p;
    }
    
    list_for_each(pos, &queueLowProc) {
        if((p = container_of(pos, pcb_t, p_list))->p_pid == pid)
            return p;
    }
    p = isPcbBlocked(pid);
    return p;
}


void block_curr_proc(state_t *excState, int *semaddr) {
    update_curr_proc_time();
    copy_state(excState, &currentActiveProc->p_s);
    insertBlocked(semaddr, currentActiveProc);
    ++blockedProc;
    scheduler();
}

// unblock process
pcb_PTR free_process(int *semaddr) {
    pcb_PTR pid = removeBlocked(semaddr);
    --blockedProc;
    insert_ready_queue(pid->p_prio, pid);
    return pid;
}

// terminate child process
void term_proc(int pid) {
    pcb_PTR p;

    if (pid == 0)
        term_proc_and_child(currentActiveProc);
    else {
        p = find_pcb(pid);
        term_proc_and_child(p);
    }
}

// return list length
int lenQ(struct list_head *l) {
    int c = 0;
    struct list_head * tmp;
    list_for_each(tmp, l) {
        ++c;
    }
    return c;
}

/* SYSCALL */

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
    if(currentActiveProc->p_prio == PROCESS_PRIO_HIGH && lenQ(&queueHighProc) == 1 && activeProc - blockedProc <= 1) //TODO : CONTROLLARE
        yieldHighProc = TRUE;  // se il processo e' ad alta priorita' e ci sono altri processi attivi, faccio lo yield in modo particolare
    scheduler();
}

