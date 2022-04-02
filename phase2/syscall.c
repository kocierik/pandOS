#include "headers/syscall.h"

/* Variabili globali esterne */
#define SEMDEVLEN 49

extern int processId;
extern int activeProc;
extern int blockedProc;
extern struct list_head queueLowProc;
extern struct list_head queueHighProc;
extern pcb_t *currentActiveProc;
extern short semDevice[SEMDEVLEN];

/* Funzioni globali esterne */
extern void assegnaPID(pcb_PTR p);
extern void insertReadyQueue(int prio, pcb_PTR p);


void createProcess() {
    pcb_PTR p = allocPcb();
    
    if(p == NULL)
        currentActiveProc->p_s.reg_v0 = -1;
    else {
        currentActiveProc->p_s.reg_v0 = p->p_pid;
        assegnaPID(p);

        insertChild(currentActiveProc,p);        
        
        p->p_s = (*(state_t *)currentActiveProc->p_s.reg_a1);
        p->p_prio = currentActiveProc->p_s.reg_a2;
        p->p_supportStruct = currentActiveProc->p_s.reg_a3;

        insertReadyQueue(p->p_prio, p);
    }
    return;
}


/* Ricerca il processo da terminare ed invoca la funzione che lo termina */
void terminateProcess(int pid) {

    int found = 0; //flag found per efficienza
    pcb_PTR p;
    struct list_head *pos;

    if (pid == 0) {
        // se il pid e' 0, allora termino il processo corrente
        __terminateProcess(currentActiveProc);
    } else {
        // senno' lo cerco nelle liste dei processi ready
        list_for_each(pos, &queueLowProc) {
            if((p = container_of(pos, pcb_t, p_list))->p_pid == pid) {
                __terminateProcess(p);
                found = 1;
            }
        }
        if(!found) {
            list_for_each(pos, &queueHighProc) {
                if((p = container_of(pos, pcb_t, p_list))->p_pid == pid)
                    __terminateProcess(p);
            }
        }
    }
}


/* funzione che libera un processo */
static void __terminateProcess(pcb_PTR p) {
    terminateDescendance(p);
    freePcb(p);
}


/* funzione ricorsiva di aiuto per terminare tutti i processi figli di un processo */
static void terminateDescendance(pcb_PTR rootPtr) {
    pcb_PTR p;
    while (!emptyChild(rootPtr)) {
        //termino il primo processo figlio
        //questa chiamata termina ricorsivamente anche i processi figlio del figlio
        terminateProcess(container_of(rootPtr->p_child.next, pcb_t, p_sib)->p_pid);
        //rimuovendo il primo figlio vado avanti con la lista dei figli
        removeChild(rootPtr);
    }
}

void passeren(int *semaddr) {
    if((*semaddr) > 0)
        --(*semaddr);
    if ((*semaddr) == 0) {
        int pid = currentActiveProc->p_pid;
        //blocca il processo
    }
}


void verhogen(int *semaddr) {
    if(list_empty(&findASL(semaddr)->s_procq))
        ++(*semaddr);
}


void doIOdevice() {

}

//da salvare in v0 quindi la funzione sarà void
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