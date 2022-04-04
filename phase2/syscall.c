#include "headers/syscall.h"

extern void klog_print(char *s);

/* Variabili globali esterne */
extern int processId;
extern struct list_head queueLowProc;
extern struct list_head queueHighProc;
extern pcb_t *currentActiveProc;
extern short semDevice[SEMDEVLEN];

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
    pcb_PTR p;
    if (*pid == 0) {
        // se il pid e' 0, allora termino il processo corrente
        __terminateProcess(callerProcess);
    } else {
        // senno' lo cerco nelle liste dei processi ready
        p = findPcb(*pid, queueLowProc);
        if(p == NULL)
            p = findPcb(*pid, queueHighProc);
        __terminateProcess(p);
    }
}


/* termina un processo */
void __terminateProcess(pcb_PTR p) {
    terminateDescendance(p);
    //gestisco il semaforo del processo
    /*
    If the value of a semaphore is negative, it is an invariant that
    the absolute value of the semaphore equal the number of pcb’s blocked on that
    semaphore. Hence if a terminated process is blocked on a semaphore,
    the value of the semaphore must be adjusted; i.e. incremented.
    */
    //aggiusto coda e variabili globali
    freePcb(p);
}


/* termina tutti i processi figli di un processo */
void terminateDescendance(pcb_PTR parent) {
    /* per non usare una funzione ricorsiva, controllo tutti i possibili pcb
       e li termino se hanno come parent il processo da terminare */

    // controllo il processo corrente
    if(currentActiveProc->p_parent == parent)
        terminateProcess(0, currentActiveProc);

    // controllo i processi nelle ready queue

    // controllo i processi bloccati dai semafori
    

}


/* cerca un pcb in una lista dato il pid e la lista in cui cercare, ritorna NULL se non lo trova */
pcb_PTR findPcb(int pid, struct list_head queue) {
    struct list_head *pos;
    pcb_PTR p;
    list_for_each(pos, &queue) {
        if((p = container_of(pos, pcb_t, p_list))->p_pid == pid)
            return p;
    }
    return NULL;
}

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
    return currentActiveProc->p_supportStruct;
}

//da salvare in v0 quindi la funzione sarà void
int getIDprocess(int parent) {
    if(parent==0)
        return currentActiveProc->p_pid;
    else
        return currentActiveProc->p_parent->p_pid;
}


void yield() {
    
}