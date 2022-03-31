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
        p->p_pid = processId++;

        insertChild(currentActiveProc,p);        
        
        p->p_s = (*(state_t *)currentActiveProc->p_s.reg_a1);
        p->p_prio = currentActiveProc->p_s.reg_a2;
        p->p_supportStruct = currentActiveProc->p_s.reg_a3;

        insertReadyQueue(p->p_prio, p);
    }
    return;
}


void terminateProcess(int pid) {
    if (pid == 0) {
        
    } else {
        
    }
}

void terminateDescendance(pcb_PTR rootPtr) {
    
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