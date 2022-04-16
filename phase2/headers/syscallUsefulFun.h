#ifndef SYSFUNCTION_H_INCLUDED
#define SYSFUNCTION_H_INCLUDED

#include "../../generic_headers/pandos_const.h"
#include "../../phase1/headers/pcb.h"
#include "../../phase1/headers/asl.h"
#include "../../phase1/headers/listx.h"
#include "umps/const.h"
#include "umps/libumps.h"
#include "umps/cp0.h"
#include "scheduler.h"
#include "handlerFunction.h"
#include "globals.h"

/* Funzioni di aiuto per syscall */
void copy_state(state_t *new, state_t *old);
void term_single_proc(pcb_PTR p);
void term_proc_and_child(pcb_PTR rootPtr);
pcb_PTR find_pcb(int pid);
void block_curr_proc(state_t *excState, int *semaddr);
pcb_PTR free_process(int *semaddr);
void term_proc(int pid);
int lenQ(struct list_head *l);

#endif