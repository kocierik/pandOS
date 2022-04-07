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
extern int semDevice[SEMDEVLEN];

/* Funzioni globali esterne */
extern void assegnaPID(pcb_PTR p);
extern void insertReadyQueue(int prio, pcb_PTR p);


void copyState(state_t *s, pcb_PTR p) {
    p->p_s.cause = s->cause;
    p->p_s.entry_hi = s->entry_hi;
    for (int i = 0; i < STATE_GPR_LEN; i++)
        p->p_s.gpr[i] = s->gpr[i];
    p->p_s.hi = s->hi;
    p->p_s.lo = s->lo;
    p->p_s.pc_epc = s->pc_epc;
    p->p_s.status = s->status;
}


int createProcess(state_t *a1, int a2, support_t *a3) {
    pcb_PTR p = allocPcb();
    
    if(p == NULL)
        return NOPROC;
    else {
        insertChild(currentActiveProc, p);
        copyState(a1, p);
        insertReadyQueue(a2, p);
        p->p_supportStruct = a3;
        return p->p_pid;
    }
}


/* Ricerca il processo da terminare ed invoca la funzione che lo termina */
int terminateProcess(int *pid) {
    int blocking_callerProc = FALSE;
    if (*pid == 0) {
        // se il pid e' 0, allora termino il processo corrente
        blocking_callerProc = TRUE;
        term_proc_and_child(currentActiveProc);
    } else {
        blocking_callerProc = term_proc_and_child(container_of(pid, pcb_t, p_pid));
    }
    return blocking_callerProc;
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
            currentActiveProc = NULL;
            ret = TRUE;
        }
        --activeProc;
    } else {
        --blockedProc;
        if (p->p_semAdd <= &(semDevice[0]) || p->p_semAdd >= &(semDevice[SEMDEVLEN-1])) {
            if((*p->p_semAdd) < 0)
                ++(*p->p_semAdd);
        }
        outBlocked(p);
    }
    
    outChild(p); // tolgo p come figlio così va avanti
    freePcb(p);
    return ret;
}

/* Porta il processo attualmente attivo in stato "Blocked" */
int passeren(int *semaddr) {

    if (*semaddr > 0) --(*semaddr);
    else {
        insertBlocked(semaddr, currentActiveProc);
        --activeProc;
        ++blockedProc;
        klog_print("\n\nP: Passeren eseguita su processo: ");
        klog_print_dec(currentActiveProc->p_pid);
        currentActiveProc = NULL; // TODO: ridondante dato il valore di return???
        return TRUE;
    }
    return FALSE;
}


/* Porta il primo processo disponibile di un semaforo dallo stato "Blocked" in "Ready" */
void verhogen(int *semaddr) {
    pcb_PTR pid = removeBlocked(semaddr);

    if (pid == NULL) ++(*semaddr);
    else {
        //Proc rimosso dal semaforo, lo inserisco nella lista dei proc ready
        --blockedProc;
        insertReadyQueue(pid->p_prio, pid);
    }
    /*
     * //NOTA BENE: //NOTA BENE: //NOTA BENE: //NOTA BENE: //NOTA BENE: //NOTA BENE: 
     *Rimuovendo questa print ti ritroverai nel bug dell'interrupt local timer loop.
    */
    klog_print("\n\nVerhogen eseguita con successo..."); 
}

int doIOdevice(int *cmdAddr, int cmdValue) {

    int deviceNumber;
    int is_recv_command = 0;
    devregarea_t *deviceRegs = (devregarea_t*) RAMBASEADDR;
    for (int i = 0; i < 8; i++){
        if (& (deviceRegs->devreg[4][i].term.transm_command) == (memaddr*) cmdAddr) 
            deviceNumber = i;
        else if (& (deviceRegs->devreg[4][i].term.recv_command) == (memaddr*) cmdAddr){
            deviceNumber = i;
            is_recv_command = 1;
        }
    }
    //Terminale che esegue la chiamata.
    termreg_t terminal = deviceRegs->devreg[4][deviceNumber].term;
    
    //Semaforo sul quale devo bloccare il processo corrente.
    int semaphoreIndex = 4 * 8 + deviceNumber*2 + is_recv_command; 

    // Eseguo il comando richiesto.
    *cmdAddr = cmdValue;

    //Eseguo la P del processo attualmente in esecuzione.
    passeren((int*) semDevice[semaphoreIndex]);

    if (is_recv_command) return terminal.recv_status;
    else return terminal.transm_status;
}


void getCpuTime() {
    cpu_t t;
    STCK(t); 
    currentActiveProc->p_s.reg_v0 = t;
}


void waitForClock() {
    passeren(&semDevice[SEMDEVLEN-1]);
    insertBlocked(&semDevice[SEMDEVLEN-1], currentActiveProc);
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
        insertReadyQueue(PROCESS_PRIO_LOW, currentActiveProc); 
    else                          
        insertReadyQueue(PROCESS_PRIO_HIGH,currentActiveProc);
}