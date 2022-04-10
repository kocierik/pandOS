#include "headers/scheduler.h"
// TEMPORARY
extern void klog_print(char *s);
extern void klog_print_dec(unsigned int num);
extern void loadState(state_t *s);

extern int activeProc;
extern int blockedProc;
extern struct list_head queueLowProc;
extern struct list_head queueHighProc;
extern pcb_t *currentActiveProc;
extern cpu_t startTime;

void scheduler() {
    pcb_PTR p;
    if((p = removeProcQ(&queueHighProc)) != NULL) {
        currentActiveProc = p;
        loadState(&p->p_s);
    } else if ((p = removeProcQ(&queueLowProc)) != NULL) {
        klog_print("\n\n sto per caricare: ");
        klog_print_dec(p->p_pid);

        currentActiveProc = p;
        setTIMER(TIMESLICE);
        loadState(&p->p_s);
    } else
        scheduler_empty_queues();
}


void scheduler_empty_queues() {
    klog_print("\n\nCode dei processi in attesa vuote...");
    if(activeProc == 0)
        HALT(); //Il processore non deve fare niente quindi si ferma
        
    if(activeProc > 0 && blockedProc > 0) {
        //Enabling interrupts and disable PLT.
        unsigned int status = IEPON | IMON;
        setSTATUS(status);
        WAIT(); //twiddling its thumbs
    }

    if(activeProc > 0 && blockedProc == 0)
        PANIC();        //DEADLOCK
}