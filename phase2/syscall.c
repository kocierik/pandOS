#include "headers/syscall.h"

extern void klog_print(char *s);
extern void klog_print_dec(unsigned int num);
extern void klog_print_hex(unsigned int num);

/* Variabili globali esterne */
extern int activeProc;
extern int blockedProc; 
extern int processId;
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

/* Funzioni globali esterne */
extern void insert_ready_queue(int prio, pcb_PTR p);


/* FUNZIONI DI AIUTO */


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


/* funzione iterativa che elimina i figli e il processo stesso */
void term_proc_and_child(pcb_PTR parent) {
    pcb_PTR p;
    while(!isPcbFree(parent->p_pid)) {
        p = parent;
        while(!emptyChild(p))
            p = container_of(p->p_child.next, pcb_t, p_sib);
        
        term_single_proc(p); // termino p
    }
}


// TODO: CONTROLLARE SE E' GIUSTA
void term_single_proc(pcb_PTR p) {
    // gestisco variabili globali e semaforo
    if (p == currentActiveProc)
        currentActiveProc = NULL;
    --activeProc;
    if (p->p_semAdd != NULL) {
        --blockedProc;
        outBlocked(p);
    }
    
    outChild(p); // tolgo p come figlio così va avanti
    list_del(&p->p_list);
    freePcb(p);
}


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


void update_curr_proc_time() {
    cpu_t now;
    STCK(now);
    currentActiveProc->p_time += now - startTime;
    STCK(startTime);
}


void block_curr_proc(state_t *excState, int *semaddr) {
    copy_state(excState, &currentActiveProc->p_s);
    insertBlocked(semaddr, currentActiveProc);
    ++blockedProc;
    scheduler();
}


void free_process(int *semaddr) {
    pcb_PTR pid = removeBlocked(semaddr);
    --blockedProc;
    insert_ready_queue(pid->p_prio, pid);
}


void term_proc(int pid) {
    pcb_PTR p;

    if (pid == 0)
        term_proc_and_child(currentActiveProc);
    else {
        p = find_pcb(pid);
        term_proc_and_child(p);
    }
}


/* SYSCALL */


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


/* Ricerca il processo da terminare ed invoca la funzione che lo termina */
void terminate_process(state_t *excState) {
    int pid = (int) (*excState).reg_a1;
    term_proc(pid);
    load_or_scheduler(excState);
}


/* Porta il processo attualmente attivo in stato "Blocked" */
void passeren(state_t *excState) {
    int *semaddr = (int*) (*excState).reg_a1;
    P(semaddr, excState);
    load_or_scheduler(excState);
}


void P(int *semaddr, state_t *excState) { //TODO passa direttametne PCB come secondo parametro dato che ha lo stato già aggiornato.
    pcb_PTR pid = headBlocked(semaddr);

    if(*semaddr == 0)
        block_curr_proc(excState, semaddr);
    else if(pid != NULL)
        free_process(semaddr);
    else
        --(*semaddr);
}


/* Porta il primo processo disponibile di un semaforo dallo stato "Blocked" in "Ready" */
void verhogen(state_t *excState) {
    int *semaddr = (int*) (*excState).reg_a1;
    V(semaddr, excState);
    load_or_scheduler(excState);
}


void V(int *semaddr, state_t *excState) {
    pcb_PTR pid = headBlocked(semaddr);

    if(*semaddr == 1)
        block_curr_proc(excState, semaddr);
    else if(pid != NULL)
        free_process(semaddr);
    else
        ++(*semaddr);
}


void do_IO_device(state_t *excState) {
    int *cmdAddr = (int*) (*excState).reg_a1;
    int cmdValue = (int)  (*excState).reg_a2;
    
    int *devSemaphore; //Indirizzo del semaforo 
    int interruptLine;
    devregarea_t *deviceRegs = (devregarea_t*) RAMBASEADDR;

    for (int i = 0; i < 8; i++){
        if (& (deviceRegs->devreg[4][i].term.transm_command) == (memaddr*) cmdAddr) { //Terminal Devices Writing
            devSemaphore = &semTerminalDeviceWriting[i];
            //returnStatus = deviceRegs->devreg[4][i].term.transm_status;
            interruptLine = 7;
            break;
        }
        else if (& (deviceRegs->devreg[4][i].term.recv_command) == (memaddr*) cmdAddr) { //Terminal Devices Reading
            devSemaphore = &semTerminalDeviceReading[i];
            //returnStatus = deviceRegs->devreg[4][i].term.recv_status;
            interruptLine = 7;
            break;
        }else{
            for(int j = 0; j < 4; j++){
                if (& (deviceRegs->devreg[j][i].dtp.command) == (memaddr*) cmdAddr ){
                    //returnStatus = deviceRegs->devreg[j][i].dtp.status;
                    if (j == 0)      { devSemaphore = &semDiskDevice[i];      interruptLine = j; }
                    else if (j == 1) { devSemaphore = &semFlashDevice[i];     interruptLine = j; }  
                    else if (j == 2) { devSemaphore = &semNetworkDevice[i];   interruptLine = j; }
                    else             { devSemaphore = &semPrinterDevice[i];   interruptLine = j; }
                }
                break;
            }
        }
    }


    (*excState).status |= STATUS_IM(interruptLine);

    copy_state(excState, &currentActiveProc->p_s);
    insertBlocked(devSemaphore, currentActiveProc);
    ++blockedProc;

    *cmdAddr = cmdValue; //appena il controllo arriva al processo corrente, dovrebbe alzarsi una interrupt
    //TODO: CAPIRE CHE CAZZO FARE QUA
    // CHIAMO LO SCHEDULER O FACCIO LDST DI QUALCOSA?????
    // COSA FA ADESO CHE NON C'E' NIENTE
}


void get_cpu_time(state_t *excState) {
    update_curr_proc_time();
    excState->reg_v0 = currentActiveProc->p_time;
    load_or_scheduler(excState);
}


void wait_for_clock(state_t *excState) {
    klog_print("\n\nsono nel wait for clock ");
    klog_print_dec(semIntervalTimer);
    P(&semIntervalTimer, excState); // questa P dovrebbe essere sempre bloccante
    load_or_scheduler(excState); // just in case
}


//TODO: sul libro non c'e' scritto esplicitamente di mettere il valore nel reg_v0, da controllare...
void get_support_data(state_t *excState) {
    (*excState).reg_v0 = (unsigned int) ((currentActiveProc->p_supportStruct != NULL) ? currentActiveProc->p_supportStruct : NULL);
    load_or_scheduler(excState);
}


void get_ID_process(state_t *excState) {
    int parent = (int) (*excState).reg_a1;
    if (parent == 0)
        (*excState).reg_v0 = currentActiveProc->p_pid;
    else
        (*excState).reg_v0 = currentActiveProc->p_parent->p_pid;

    load_or_scheduler(excState);
}


// inserisce il processo chiamante al termine della coda della rispettiva coda dei processi
void yield(state_t *excState) {
    copy_state(excState, &currentActiveProc->p_s);
    insert_ready_queue(currentActiveProc->p_prio, currentActiveProc);
    if(currentActiveProc->p_prio == PROCESS_PRIO_HIGH) yieldHighProc = TRUE;
    scheduler();
}