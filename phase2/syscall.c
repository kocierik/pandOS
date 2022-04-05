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
        while(!emptyChild(p))
            p = container_of(p->p_child.next, pcb_t, p_sib);
        
        __terminate_process(p); // termino p
    }
}


void __terminate_process(pcb_PTR p) {
    // gestisco variabili globali e semaforo
    if (p->p_semAdd == NULL) {
        list_del(&p->p_list);   // lo tolgo da qualsiasi lista
        if (p == currentActiveProc)
            currentActiveProc = NULL;
        --activeProc;
    } else {
        --blockedProc;
        outBlocked(p);
        if (p->p_semAdd <= &(semDevice[0]) || p->p_semAdd >= &(semDevice[SEMDEVLEN-1])) {
            if((*p->p_semAdd) < 0)
                ++(*p->p_semAdd);
        } else {
            // TODO?? When the interrupt eventually occurs the semaphore
            // will get V’ed (and hence incremented) by the interrupt handler.
        }
    }
    
    outChild(p); // tolgo p come figlio così va avanti
    freePcb(p);
}


/*
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
    --(*semaddr);

    if (*semaddr <= 0) {
        insertBlocked(semaddr, currentActiveProc);
        --activeProc;
        ++blockedProc;
        currentActiveProc = NULL; // Il processo che prima era attivo ora non lo è più.
    }
    klog_print("Chiamata ed eseguita passeren\n\n");
}


/* Porta il primo processo disponibile di un semaforo dallo stato "Blocked" in "Ready" */
void verhogen(int *semaddr) {
    ++(*semaddr);
    pcb_PTR pid = removeBlocked(semaddr);
    if (pid != NULL) {
        //Proc rimosso dal semaforo, lo inserisco nella lista dei proc ready
        --blockedProc;
        insertReadyQueue(pid->p_prio, pid);
    }
    klog_print("Chiamata ed eseguita verhogen\n\n");
}


int doIOdevice(int *cmdAddr, int cmdValue, pcb_PTR callerProcess) {
    //Current process da running state va in blocked state
    /*
    Dunque devo eseguire una P sul processo corrente.
    Ma con quale semaforo chiamo la P?
    "P operation on the semaphore that
    the Nucleus maintains for the I/O device indicated by the values in a1, a2,
    and optionally a3."
    */
    int ret = insertBlocked(NULL, callerProcess);
    return ret;
}


void getCpuTime() {
    cpu_t t;
    STCK(t); 
    currentActiveProc->p_s.reg_v0 = t;
}


void waitForClock(pcb_PTR callerProccess) {
    passeren(&semDevice[SEMDEVLEN-1]);
    insertBlocked(&semDevice[SEMDEVLEN-1], callerProccess);
    --activeProc;
    ++blockedProc;
}


support_t* getSupportData() {
    return (currentActiveProc->p_supportStruct != NULL) ? currentActiveProc->p_supportStruct : NULL;
}


void getIDprocess(pcb_PTR callerProcess, int parent) {
    if(parent==0)
        callerProcess->p_s.reg_v0 = callerProcess->p_pid;
    else
        callerProcess->p_s.reg_v0 = callerProcess->p_parent->p_pid;
}


// inserisce il processo chiamante al termine della coda della rispettiva coda dei processi
void yield() {
    list_del(&currentActiveProc->p_list);

    if(currentActiveProc->p_prio == PROCESS_PRIO_LOW)
        insertReadyQueue(PROCESS_PRIO_LOW, currentActiveProc); 
    else                          
        insertReadyQueue(PROCESS_PRIO_HIGH,currentActiveProc);
}