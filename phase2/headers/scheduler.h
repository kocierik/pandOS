#ifndef SCHEDULER_H_INCLUDED
#define SCHEDULER_H_INCLUDED

#include "../../phase1/headers/pcb.h"
#include "../../generic_headers/pandos_const.h"
#include "umps/libumps.h"
#include "globals.h"

extern void insert_ready_queue(int prio, pcb_PTR p);

void scheduler();
void load_state(state_t *s);
void load_or_scheduler(state_t *s);
void update_curr_proc_time();
void freeYieldHP();
void scheduler_empty_queues();

#endif