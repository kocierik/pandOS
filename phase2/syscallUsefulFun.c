#include "headers/syscallUsefulFun.h"


/* Useful function for Syscall handling */

void copy_state(state_t *new, state_t *old) {
    old->cause = new->cause;
    old->entry_hi = new->entry_hi;
    for (int i = 0; i < STATE_GPR_LEN; i++)
        old->gpr[i] = new->gpr[i];
    old->hi = new->hi;
    old->lo = new->lo;
    old->pc_epc = new->pc_epc;
    old->status = new->status;
}

// Remove child process
void term_proc_and_child(pcb_PTR parent) {
    pcb_PTR p;
    while(!isPcbFree(parent->p_pid)) {
        p = parent;
        while(!emptyChild(p))
            p = container_of(p->p_child.next, pcb_t, p_sib);
        
        term_single_proc(p); // ending P
    }
}


void term_single_proc(pcb_PTR p) {
    --activeProc;
    if (p->p_semAdd != NULL) {
        --blockedProc;
        outBlocked(p);
    }    
    if (p == currentActiveProc)
        currentActiveProc = NULL;
    
    outChild(p); // take away p as son so it goes on
    freePcb(p);
}

// Find the blocking pcb in the two queues
pcb_PTR find_pcb(int pid) {
    struct list_head *pos;
    pcb_PTR p;

    list_for_each(pos, &queueHighProc) {
        if((p = container_of(pos, pcb_t, p_list))->p_pid == pid)
            return p;
    }
    
    list_for_each(pos, &queueLowProc) {
        if((p = container_of(pos, pcb_t, p_list))->p_pid == pid)
            return p;
    }
    p = isPcbBlocked(pid);
    return p;
}


void block_curr_proc(state_t *excState, int *semaddr) {
    update_curr_proc_time();
    copy_state(excState, &currentActiveProc->p_s);
    insertBlocked(semaddr, currentActiveProc);
    ++blockedProc;
    scheduler();
}

// unblock process
pcb_PTR free_process(int *semaddr) {
    pcb_PTR pid = removeBlocked(semaddr);
    --blockedProc;
    insert_ready_queue(pid->p_prio, pid);
    return pid;
}

// terminate child process
void term_proc(int pid) {
    pcb_PTR p;

    if (pid == 0)
        term_proc_and_child(currentActiveProc);
    else {
        p = find_pcb(pid);
        term_proc_and_child(p);
    }
}

// return list length
int lenQ(struct list_head *l) {
    int c = 0;
    struct list_head * tmp;
    list_for_each(tmp, l) {
        ++c;
    }
    return c;
}