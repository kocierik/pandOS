#ifndef SYSCALL_H_INCLUDED
#define SYSCALL_H_INCLUDED

#include "../../generic_headers/pandos_const.h"
#include "../../phase1/headers/pcb.h"
#include "../../phase1/headers/asl.h"
#include "../../phase1/headers/listx.h"
#include "umps/const.h"
#include "umps/libumps.h"
#include "umps/cp0.h"
#include "scheduler.h"


/* Funzioni di aiuto per syscall */
void copy_state(state_t *new, state_t *old);
int term_single_proc(pcb_PTR p);
int term_proc_and_child(pcb_PTR rootPtr);
pcb_PTR find_pcb(int pid);
void update_curr_proc_time();
void block_curr_proc(state_t *excState, int *semaddr);
void free_process(int *semaddr);
void P(int *semaddr, state_t *excState);


void create_process(state_t *excState);
void terminate_process(state_t *excState);
void passeren(state_t *excState);
void verhogen(state_t *excState);
void do_IO_device(state_t *excState);
void getCpuTime(state_t *excState);
void waitForClock(state_t *excState);
void getSupportData(state_t *excState);
void getIDprocess(state_t *excState);
void yield(state_t *excState);


#endif