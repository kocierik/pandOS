#include "headers/syscall.h"

extern void klog_print(char *s);

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


void copyState(state_t * s, pcb_PTR p) {
    p->p_s.cause = s->cause;
    p->p_s.entry_hi = s->entry_hi;
    for (int i = 0; i < STATE_GPR_LEN; i++) {
        p->p_s.gpr[i] = s->gpr[i];
    }
    p->p_s.hi = s->hi;
    p->p_s.lo = s->lo;
    p->p_s.pc_epc = s->pc_epc;
    p->p_s.status = s->status;
}


void createProcess(state_t * callerProcState) {
    
    pcb_PTR p = allocPcb();
    
    if(p == NULL)
        (*callerProcState).reg_v0 = NOPROC;
    else {
        assegnaPID(p);
        (*callerProcState).reg_v0 = p->p_pid;

        insertChild(container_of(callerProcState, pcb_t, p_s), p);
          
        copyState((state_t *)(*callerProcState).reg_a1,p);
        insertReadyQueue((*callerProcState).reg_a2, p);
        p->p_supportStruct = (support_t *)(*callerProcState).reg_a3;
    }
    // control is returned to the Current Process. [Section 3.5.12] 
    LDST(callerProcState); //TODO: CONTROLLARE
    /*
    qui bisognerebbe capire meglio come gestircela visto che bisogna distinguere tra
    syscall BLOCCANTI e syscall NON BLOCCANTI :
    le syscall NON BLOCCANTI continuano l'esecuzione del processo chiamante,
    invece le syscall BLOCCANTI fanno delle altre robe e chiamano poi lo scheduler
    MAGARI CE LO GESTIAMO NEL SYSCALL_HANDLER?? invece che nelle singole syscall,
    Per ora lasciamo quel LDST così...
    by geno
    */
}


/* Ricerca il processo da terminare ed invoca la funzione che lo termina */
void terminateProcess(int *pid, pcb_PTR callerProcess) {
    if (*pid == 0) {
        // se il pid e' 0, allora termino il processo corrente
        term_proc_and_child(callerProcess);
    } else {
        term_proc_and_child(container_of(pid, pcb_t, p_pid));
    }
}


/* funzione iterativa che elimina i figli e il processo stesso */
void term_proc_and_child(pcb_PTR parent) {
    pcb_PTR p;
    while(!isPcbFree(parent->p_pid)) {
        p = parent;
        while(!emptyChild(p)) {
            p = container_of(p->p_child.next, pcb_t, p_sib);
        }
        
        __terminate_process(p); // termino p
    }
}


void __terminate_process(pcb_PTR p) {
    if (p->p_semAdd == NULL) {
        list_del(&p->p_list);   // lo tolgo da qualsiasi lista
        --activeProc;
    } else {
        --blockedProc;
        if (p->p_semAdd >= &(semDevice[0]) && p->p_semAdd <= &(semDevice[SEMDEVLEN-1]))
            ++(*p->p_semAdd);
        outBlocked(p);
        
    }
    
    outChild(p); // tolgo p come figlio così va avanti
    freePcb(p);
}

/* ok penso di avere un problema con l'inclusione della libreria di umps3 e quindi mi da errore nell'avvio del kernel...
int lenQ(struct list_head queue) {
    struct list_head *pos;
    int c = 0;
    list_for_each(pos, &queue) {
        ++c;
    }
    return c;
}
*/


/* cerca un pcb in una lista dato il pid e la lista in cui cercare, ritorna NULL se non lo trova */
/*
pcb_PTR findPcb(int pid, struct list_head queue) {
    struct list_head *pos;
    pcb_PTR p;
    list_for_each(pos, &queue) {
        if((p = container_of(pos, pcb_t, p_list))->p_pid == pid)
            return p;
    }
    return NULL;
}
*/

/* Porta il processo attualmente attivo in stato "Blocked" */
void passeren(int *semaddr) {
    if (*semaddr > 0) (*semaddr) --;
    else{
        pcb_t *pid = currentActiveProc;
        insertBlocked(semaddr, pid);
        currentActiveProc = NULL; // Il processo che prima era attivo ora non lo è più.
    }
    klog_print("Chiamata ed eseguita passeren\n\n");
}

/* Porta il primo processo disponibile di un semaforo dallo stato "Blocked" in "Ready" */
void verhogen(int *semaddr) {
    pcb_t *pid = removeBlocked(semaddr);
    if (pid == NULL){   //Non vi è alcun processo da rimuovere
        klog_print("Nessun processo da rimuovere\n\n");
        (*semaddr) ++;
    }else{ //Proc rimosso dal semaforo, lo inserisco nella lista dei proc ready
        if (pid->p_prio){ //Proc ad alta priorità
            insertProcQ(&queueHighProc, pid);
        }else{ //Proc a bassa priorità
            insertProcQ(&queueLowProc, pid);
        }        
    }
    klog_print("Chiamata ed eseguita verhogen\n\n");
}


int doIOdevice(int *cmdAddr, int cmdValue) {
    //Current process da running state va in blocked state
    /*
    Dunque devo eseguire una P sul processo corrente.
    Ma con quale semaforo chiamo la P?
    "P operation on the semaphore that
    the Nucleus maintains for the I/O device indicated by the values in a1, a2,
    and optionally a3."
    */
}


void getCpuTime() {
    cpu_t t;
    STCK(t);
    currentActiveProc->p_s.reg_v0 = t;
}


void waitForClock() {

}


support_t* getSupportData() {
    return (currentActiveProc->p_supportStruct != NULL) ? currentActiveProc->p_supportStruct : NULL;
}

//da salvare in v0 quindi la funzione sarà void
int getIDprocess(int parent) {
    if(parent==0)
        return currentActiveProc->p_pid;
    else
        return currentActiveProc->p_parent->p_pid;
}

void yield(){ 
    list_del(currentActiveProc->p_list);
    if(currentActiveProc->p_prio == PROCESS_PRIO_LOW){ 
        insertReadyQueue(PROCESS_PRIO_LOW, currentActiveProc); 
    }else{                                              
        insertReadyQueue(PROCESS_PRIO_HIGH,currentActiveProc);
    }
}