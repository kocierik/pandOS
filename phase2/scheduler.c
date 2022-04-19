#include "headers/scheduler.h"

/* External usefull variables */
extern int activeProc;
extern int blockedProc;
extern struct list_head queueLowProc;
extern struct list_head queueHighProc;
extern pcb_t *currentActiveProc;
extern cpu_t startTime;
extern int yieldHighProc;


/**
 * If there are processes in the high priority queue, remove the first one and load its state;
 * otherwise, if there are processes in the low priority queue, remove the first one and load its state;
 * otherwise call scheduler_empty_queues()
 */
void scheduler()
{
    pcb_PTR p;

    if (currentActiveProc != NULL)
        update_curr_proc_time();

    if ((p = removeProcQ(&queueHighProc)) != NULL && !yieldHighProc)
    {
        currentActiveProc = p;
        load_state(&p->p_s);
    }
    else if ((p = removeProcQ(&queueLowProc)) != NULL)
    {
        if (yieldHighProc)
            yieldHighProc = FALSE;

        currentActiveProc = p;
        setTIMER(TIMESLICE); // PLT 5 ms
        load_state(&p->p_s);
    }
    else
        scheduler_empty_queues();
}

/**
 * It loads the state of the current active process, or if there is no current active process, it calls
 * the scheduler
 * 
 * @param s the state to load
 */
void load_or_scheduler(state_t *s)
{
    if (currentActiveProc == NULL)
        scheduler();
    load_state(s);
}

/**
 * It loads the state from the stack into the registers
 * 
 * @param s the state to load
 */
void load_state(state_t *s)
{
    STCK(startTime); // start timer
    LDST(s);
}

/**
 * It updates the current process's time by adding the time between the last time it was called and the
 * current time
 */
void update_curr_proc_time()
{
    cpu_t now;
    STCK(now); // stop timer
    currentActiveProc->p_time += now - startTime;
    STCK(startTime);
}

/**
 * If there are no active processes, halt the processor. If there are active processes and blocked
 * processes, enable interrupts and disable PLT. If there are active processes and no blocked
 * processes, panic
 */
void scheduler_empty_queues()
{
    if (activeProc == 0)
        HALT(); // halt processor

    if (activeProc > 0 && blockedProc > 0)
    {
        // Enabling interrupts and disable PLT.
        setTIMER(-2);
        unsigned int status = IECON | IMON;
        currentActiveProc = NULL;
        setSTATUS(status);
        WAIT(); // twiddling its thumbs
    }

    if (activeProc > 0 && blockedProc == 0)
        PANIC(); // DEADLOCK
}