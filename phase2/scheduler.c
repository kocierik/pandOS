#include "headers/scheduler.h"

extern int activeProc;
extern int blockedProc;
extern struct list_head queueLowProc;
extern struct list_head queueHighProc;
extern pcb_t *currentActiveProc;
extern cpu_t startTime;
extern int yieldHighProc;


void scheduler() {
    pcb_PTR p;

    if (currentActiveProc != NULL)
        update_curr_proc_time();

    if((p = removeProcQ(&queueHighProc)) != NULL && !yieldHighProc) {
        currentActiveProc = p;
        load_state(&p->p_s);
    } else if ((p = removeProcQ(&queueLowProc)) != NULL) {
        if (yieldHighProc) yieldHighProc = FALSE;

        currentActiveProc = p;
        setTIMER(TIMESLICE); // PLT 5 ms
        load_state(&p->p_s);
    } else
        scheduler_empty_queues();
}


void load_or_scheduler(state_t *s) {
    if(currentActiveProc == NULL) scheduler();
    load_state(s);
}


void load_state(state_t *s) {
    STCK(startTime); // start timer
    LDST(s);
}


void update_curr_proc_time() {
    cpu_t now;
    STCK(now);   // stop timer
    currentActiveProc->p_time += now - startTime;
    STCK(startTime); 
}


void scheduler_empty_queues() {
    if(activeProc == 0)
        HALT(); // halt processor
        
    if(activeProc > 0 && blockedProc > 0) {
        //Enabling interrupts and disable PLT.
        setTIMER(-2);
        unsigned int status = IECON | IMON;
        currentActiveProc = NULL;
        setSTATUS(status);
        WAIT(); //twiddling its thumbs
    }

    if(activeProc > 0 && blockedProc == 0) {
        PANIC();        //DEADLOCK
    }
}