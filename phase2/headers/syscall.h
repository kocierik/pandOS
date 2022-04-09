#ifndef SYSCALL_H_INCLUDED
#define SYSCALL_H_INCLUDED

#include "../../generic_headers/pandos_const.h"
#include "../../phase1/headers/pcb.h"
#include "../../phase1/headers/asl.h"
#include "../../phase1/headers/listx.h"
#include "umps/const.h"
#include "umps/libumps.h"
#include "umps/cp0.h"


/* Funzioni di aiuto per syscall */
void copy_state(state_t *new, state_t *old);
int __terminate_process(pcb_PTR p);
int term_proc_and_child(pcb_PTR rootPtr);
pcb_PTR findPcb(int pid);
void update_curr_proc_time();


int createProcess(state_t *a1, int a2, support_t *a3);
int terminateProcess(int pid);
int passeren(int *semaddr);
void verhogen(int *semaddr);
void doIOdevice(int *cmdAddr, int cmdValue);
void getCpuTime(state_t *callerProcState);
void waitForClock();
support_t* getSupportData();
void getIDprocess(state_t *callerProcess, int parent);
void yield();


#endif