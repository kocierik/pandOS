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


void createProcess(state_t * callerProcess) {
    /*
    pcb_PTR p = allocPcb();
    
    if(p == NULL)
        (*callerProcess).reg_v0 = -1;
    else {
        (*callerProcess).reg_v0 = p->p_pid;
        assegnaPID(p);

        insertChild(container_of(callerProcess, pcb_t, p_s), p);        
        
        p->p_s = (*(state_t *)(*callerProcess).reg_a1);
        p->p_prio = (*callerProcess).reg_a2;
        p->p_supportStruct = (support_t *)(*callerProcess).reg_a3;

        insertReadyQueue(p->p_prio, p);
    }
    */
}

/* cerca un pcb in una lista dato il pid e la lista in cui cercare*/
pcb_PTR findPcb(int *pid, struct list_head queue) {
    struct list_head *pos;
    pcb_PTR p;
    list_for_each(pos, &queue) {
        if(&((p = container_of(pos, pcb_t, p_list))->p_pid) == pid)
            return p;
    }
    return NULL;
}

/* Ricerca il processo da terminare ed invoca la funzione che lo termina */
void terminateProcess(int *pid) {
    pcb_PTR p;
    if (pid == 0) {
        // se il pid e' 0, allora termino il processo corrente
        __terminateProcess(currentActiveProc);
    } else {
        // senno' lo cerco nelle liste dei processi ready
        p = findPcb(pid, queueLowProc);
        if(p == NULL)
            p = findPcb(pid, queueHighProc);
        __terminateProcess(p);
    }
}


/* funzione ricorsiva di aiuto per terminare tutti i processi figli di un processo */
void terminateDescendance(pcb_PTR rootPtr) {
    while (!emptyChild(rootPtr)) {
        //termino il primo processo figlio
        //questa chiamata termina ricorsivamente anche i processi figlio del figlio
        terminateProcess(&(container_of(rootPtr->p_child.next, pcb_t, p_sib)->p_pid));
        //rimuovendo il primo figlio vado avanti con la lista dei figli
        removeChild(rootPtr);
    }
}


/* funzione che libera un processo */
void __terminateProcess(pcb_PTR p) {
    terminateDescendance(p);
    freePcb(p);
}



void passeren(int *semaddr) {
    if((*semaddr) > 0)
        --(*semaddr);
    if ((*semaddr) == 0) {
        //int pid = currentActiveProc->p_pid;
        //blocca il processo
    }
}


void verhogen(int *semaddr) {
    //if(list_empty(&findASL(semaddr)->s_procq))
    //    ++(*semaddr);
    klog_print("sono nel verhogen");
}


void doIOdevice(int *cmdAddr, int cmdValue) {

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