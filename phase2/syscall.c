#include "headers/syscall.h"

#define SEMDEVLEN 49

extern int processId;
extern int activeProc;
extern int blockedProc;
extern struct list_head queueLowProc;
extern struct list_head queueHighProc;
extern pcb_t *currentActiveProc;
extern short semDevice[SEMDEVLEN];

extern void assegnaPID(pcb_PTR p);
extern void insertReadyQueue(int prio, pcb_PTR p);

void createProcess() {
    pcb_PTR p = allocPcb();
    
    if(p == NULL) {
        currentActiveProc->p_s.reg_v0 = -1;
    }
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


void terminateProcess(int pid) {

    int found = 0; //flag found per efficienza
    pcb_PTR p;

    if (pid == 0) {
        terminateDescendance(currentActiveProc);
        freePcb(currentActiveProc);
    } else {
        struct list_head *pos;
        list_for_each(pos, queueLowProc) {
            if((p = container_of(pos, pcb_t, p_list))->p_pid == pid) {
                terminateDescendance(p);
                freePcb(pos);
                found = 1;
            }
        }
        if(!found) {
            list_for_each(pos, queueHighProc) {
                if((p = container_of(pos, pcb_t, p_list))->p_pid == pid) {
                    terminateDescendance(p);
                    freePcb(pos);
                }
            }
        }
    }
}

/* funzione ricorsiva di aiuto per terminare tutti i processi figli di un processo */
static void terminateDescendance(pcb_PTR rootPtr) {
    pcb_PTR p;
    while (!emptyChild(rootPtr)) {
        //termino il primo processo figlio
        //questa chiamata termina ricorsivamente anche i processi figlio del figlio
        terminateProcess(container_of(rootPtr->p_child->next, pbc_t, p_sib)->p_pid);
        //rimuovendo il primo figlio vado avanti con la lista dei figli
        removeChild(rootPtr);
    }
}

void passeren() {

}


void verhogen() {

}


void doIOdevice() {

}


void getCpuTime() {

}


void waitForClock() {

}


void getSupportData() {

}


void getIDprocess() {

}


void yield() {
    
}