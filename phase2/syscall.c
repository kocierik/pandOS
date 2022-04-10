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
int term_proc_and_child(pcb_PTR parent) {
    int ret = FALSE;
    pcb_PTR p;
    while(!isPcbFree(parent->p_pid)) {
        p = parent;
        while(!emptyChild(p))
            p = container_of(p->p_child.next, pcb_t, p_sib);
        
        ret = term_single_proc(p); // termino p
    }
    return ret;
}


int term_single_proc(pcb_PTR p) {
    int ret = FALSE;
    // gestisco variabili globali e semaforo
    if (p->p_semAdd == NULL) {
        list_del(&p->p_list);   // lo tolgo da qualsiasi lista
        if (p == currentActiveProc) {
            //currentActiveProc = NULL;
            ret = TRUE;
        }
        --activeProc;
    } else {
        --blockedProc;
        outBlocked(p);
    }
    
    outChild(p); // tolgo p come figlio così va avanti
    freePcb(p);
    return ret;
}


pcb_PTR find_pcb(int pid) {
    
    struct list_head *pos;
    pcb_PTR p;
    if (pid == currentActiveProc->p_pid)
        return currentActiveProc;

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
    ++blockedProc;
    --activeProc;
    copy_state(excState, &currentActiveProc->p_s);
    insertBlocked(semaddr, currentActiveProc);
    scheduler();
}


void free_process(int *semaddr) {
    pcb_PTR pid = removeBlocked(semaddr);
    --blockedProc;
    insert_ready_queue(pid->p_prio, pid);
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
        insert_ready_queue(a2, p);
        p->p_supportStruct = a3;
        if (a3 == 0)
            p->p_supportStruct = NULL;
        (*excState).reg_v0 = p->p_pid;
    }
}


/* Ricerca il processo da terminare ed invoca la funzione che lo termina */
/* DA DEBUGGARE */
void terminate_process(state_t *excState) {
    pcb_PTR p;
    int pid = (int) (*excState).reg_a1;

    if (pid == 0) {
        // se il pid e' 0, allora termino il processo corrente
        klog_print("\n\nsto per perminare il processo corrente");
        term_proc_and_child(currentActiveProc);
        klog_print("\n\nho terminato");
    } else {
        klog_print("\n\ndevo cercare il processo");
        p = find_pcb(pid);
        klog_print("\n\nprocesso trovato! Lo termino");
        term_proc_and_child(p);
    }
}


/* Porta il processo attualmente attivo in stato "Blocked" */
void passeren(state_t *excState) {
    int *semaddr = (int*) (*excState).reg_a1;
    P(semaddr, excState);
}


void P(int *semaddr, state_t *excState) {
    pcb_PTR pid = headBlocked(semaddr);

    if(*semaddr == 0) {
        block_curr_proc(excState, semaddr);
    } else if(pid != NULL) {
        free_process(semaddr);
    } else
        --(*semaddr);
}


/* Porta il primo processo disponibile di un semaforo dallo stato "Blocked" in "Ready" */
void verhogen(state_t *excState) {
    int *semaddr = (int*) (*excState).reg_a1;
    pcb_PTR pid = headBlocked(semaddr);

    if(*semaddr == 1) {
        block_curr_proc(excState, semaddr);
    } else if(pid != NULL) {
        free_process(semaddr);
    } else
        ++(*semaddr);
}


void do_IO_device(state_t *excState) {
    int *cmdAddr = (int*) (*excState).reg_a1;
    int cmdValue = (int)  (*excState).reg_a2;
    /*
    int deviceNumber;
    int is_terminal = 0; //Se 1 è terminal Writing, se 2 è terminal Reading, se 0 other devices.
    devreg_t callingDevice;
    */
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



    //Eseguo la P del processo attualmente in esecuzione.
    //passeren(&semTerminalDeviceWriting[1]); for debug purpose
    
    //faccio una P()
    insertBlocked(devSemaphore, currentActiveProc);
    ++blockedProc;
    --activeProc;
    // al posto della p faccio così perché ho cambiato un po' di cose e quindi meglio fare così

    //setSTATUS(IEPON);
    currentActiveProc->p_s.status |= STATUS_IM(interruptLine); 
/*
    termreg_t *devRegAddr = (termreg_t *) (0x10000054 + ((interruptLine - 3) * 0x80) + (deviceNumber * 0x10));
    klog_print("\n\ntrovato da me: ");
    klog_print_dec((memaddr*) &devRegAddr->transm_command); 
    klog_print("\n\npreso da lui: ");
    klog_print_dec((memaddr*) (cmdAddr));
*/
    // Eseguo il comando richiesto.
    *cmdAddr = cmdValue; //appena il controllo arriva al processo corrente, dovrebbe alzarsi una interrupt
}


void getCpuTime(state_t *excState) {
    update_curr_proc_time();
    excState->reg_v0 = currentActiveProc->p_time;
}


void waitForClock(state_t *excState) {
    P(&semIntervalTimer, excState);
    block_curr_proc(excState, &semIntervalTimer);
}


//TODO: sul libro non c'e' scritto esplicitamente di mettere il valore nel reg_v0, da controllare...
void getSupportData(state_t *excState) {
    (*excState).reg_v0 = (unsigned int) ((currentActiveProc->p_supportStruct != NULL) ? currentActiveProc->p_supportStruct : NULL);
}


void getIDprocess(state_t *excState) {
    int parent = (int) (*excState).reg_a1;
    if (parent == 0)
        (*excState).reg_v0 = currentActiveProc->p_pid;
    else
        (*excState).reg_v0 = currentActiveProc->p_parent->p_pid;
}


// inserisce il processo chiamante al termine della coda della rispettiva coda dei processi
void yield(state_t *excState) {
    copy_state(excState, &currentActiveProc->p_s);
    insert_ready_queue(PROCESS_PRIO_LOW, currentActiveProc); 
    --activeProc; //NON TOCCARE: da fare visto che lo faccio a spropostito in insert_ready_queue
    scheduler();
}