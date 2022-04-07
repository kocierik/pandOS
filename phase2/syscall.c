#include "headers/syscall.h"

extern void klog_print(char *s);
extern void klog_print_dec(unsigned int num);

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

/* Funzioni globali esterne */
extern void assegnaPID(pcb_PTR p);
extern void insert_ready_queue(int prio, pcb_PTR p);


/* FUNZIONI DI AIUTO */


void copyState(state_t *new, state_t *old) {
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
        
        ret = __terminate_process(p); // termino p
    }
    return ret;
}


int __terminate_process(pcb_PTR p) {
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


pcb_PTR findPcb(int pid) {
    
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


/* SYSCALL */


int createProcess(state_t *a1, int a2, support_t *a3) {
    pcb_PTR p = allocPcb();
    
    if(p == NULL)
        return NOPROC;
    else {
        insertChild(currentActiveProc, p);
        copyState(a1, &p->p_s);
        insert_ready_queue(a2, p);
        if(a3 != NULL || a3 != 0)
            p->p_supportStruct = a3;
        else
            p->p_supportStruct = NULL;
        return p->p_pid;
    }
}


/* Ricerca il processo da terminare ed invoca la funzione che lo termina */
int terminateProcess(int pid) {
    int blocking_callerProc = FALSE;
    pcb_PTR p;
    if (pid == 0) {
        // se il pid e' 0, allora termino il processo corrente
        blocking_callerProc = TRUE;
        term_proc_and_child(currentActiveProc);
    } else {
        p = findPcb(pid);
        blocking_callerProc = term_proc_and_child(p);
    }
    return blocking_callerProc;
}


/* Porta il processo attualmente attivo in stato "Blocked" */
int passeren(int *semaddr) {
    if (*semaddr > 0)
        --(*semaddr);
    else {
        insertBlocked(semaddr, currentActiveProc);
        --activeProc;
        ++blockedProc;
        klog_print("\n\nP: Passeren eseguita su processo: ");
        klog_print_dec(currentActiveProc->p_pid);
        return TRUE;
    }
    return FALSE;
}


/* Porta il primo processo disponibile di un semaforo dallo stato "Blocked" in "Ready" */
void verhogen(int *semaddr) {
    pcb_PTR pid = removeBlocked(semaddr);

    if (pid == NULL)
        ++(*semaddr);
    else {
        //Proc rimosso dal semaforo, lo inserisco nella lista dei proc ready
        --blockedProc;
        insert_ready_queue(pid->p_prio, pid);
    }
    //klog_print("\n\nVerhogen eseguita con successo..."); 
}


int doIOdevice(int *cmdAddr, int cmdValue) {

    int deviceNumber;
    int is_terminal = 0; //Se 1 è terminal Writing, se 2 è terminal Reading, se 0 other devices.
    devreg_t callingDevice;
    int *devSemaphore; //Indirizzo del semaforo 
    int returnStatus;
    devregarea_t *deviceRegs = (devregarea_t*) RAMBASEADDR;

    for (int i = 0; i < 8; i++){
        if (& (deviceRegs->devreg[4][i].term.transm_command) == (memaddr*) cmdAddr){ //Terminal Devices Writing
            devSemaphore = &semTerminalDeviceWriting[i];
            returnStatus = deviceRegs->devreg[4][i].term.transm_status;
            break;
        }
        else if (& (deviceRegs->devreg[4][i].term.recv_command) == (memaddr*) cmdAddr){ //Terminal Devices Reading
            devSemaphore = &semTerminalDeviceReading[i];
            returnStatus = deviceRegs->devreg[4][i].term.recv_status;
            break;
        }
        for(int j = 0; j < 4; j++){
            if (& (deviceRegs->devreg[j][i].dtp.command) == (memaddr*) cmdAddr ){
                returnStatus = deviceRegs->devreg[j][i].dtp.status;
                if (j == 0)      devSemaphore = &semDiskDevice[i];
                else if (j == 1) devSemaphore = &semFlashDevice[i];
                else if (j == 2) devSemaphore = &semNetworkDevice[i];
                else             devSemaphore = &semPrinterDevice[i];
            }
            break;
        }
    }

    //Terminale che esegue la chiamata.
    //termreg_t *terminal = (0x10000054 + (4 * 0x80) + (deviceNumber * 0x10));
    //termreg_t terminal = deviceRegs->devreg[4][deviceNumber].term;
    
    //Semaforo sul quale devo bloccare il processo corrente.
    //int semaphoreIndex = 4 * 8 + deviceNumber*2 + is_recv_command; 

    //Eseguo la P del processo attualmente in esecuzione.
    passeren(devSemaphore);
    currentActiveProc->p_s.status |= STATUS_IM(IEPON); //TODO INSERIRE INTERRUPT LINE AL POSTO DI IEPON
    // Eseguo il comando richiesto.
    *cmdAddr = cmdValue;
    //terminal->transm_command = cmdValue;

    
    return returnStatus;
}


void getCpuTime(state_t *excState) {
    cpu_t t;
    STCK(t);
    //currentActiveProc->p_time += (t - startT);
    excState->reg_v0 = currentActiveProc->p_time;
    //La load la faccio dopo
    /*
     * cpu_t currTime;
     * STCK(currTime); //tempo attuale
     * curr_proc->p_time += (currTime - startTod); //aggiorno il tempo
     * statep->reg_v0 = curr_proc->p_time; //preparo il regitro di ritorno
     * STCK(startTod); //faccio ripartire il "cronometro"
     * LDST(statep);
    */
}


void waitForClock() {
    passeren(&semIntervalTimer);
    insertBlocked(&semIntervalTimer, currentActiveProc);
    --activeProc;
    ++blockedProc;
}


support_t* getSupportData() {
    return (currentActiveProc->p_supportStruct != NULL) ? currentActiveProc->p_supportStruct : NULL;
}


void getIDprocess(state_t *callerProcess, int parent) {
    if(parent==0)
        (*callerProcess).reg_v0 = currentActiveProc->p_pid;
    else
        (*callerProcess).reg_v0 = currentActiveProc->p_parent->p_pid;
}


// inserisce il processo chiamante al termine della coda della rispettiva coda dei processi
void yield() {
    list_del(&currentActiveProc->p_list);

    if(currentActiveProc->p_prio == PROCESS_PRIO_LOW)
        insert_ready_queue(PROCESS_PRIO_LOW, currentActiveProc); 
    else                          
        insert_ready_queue(PROCESS_PRIO_HIGH,currentActiveProc);
}