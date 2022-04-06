#ifndef SYSCALL_H_INCLUDED
#define SYSCALL_H_INCLUDED

#include "../../generic_headers/pandos_const.h"
#include "../../phase1/headers/pcb.h"
#include "../../phase1/headers/asl.h"
#include "../../phase1/headers/listx.h"
#include "umps/const.h"
#include "umps/libumps.h"


/* Funzioni di aiuto */
int __terminate_process(pcb_PTR p);
int term_proc_and_child(pcb_PTR rootPtr);
//int lenQ(struct list_head queue);
//pcb_PTR findPcb(int pid, struct list_head queue);


void createProcess(state_t *callerProcess);
int terminateProcess(int *pid);
int passeren(int *semaddr);
void verhogen(int *semaddr);
int doIOdevice(int *cmdAddr, int cmdValue);
void getCpuTime();
void waitForClock();
support_t* getSupportData();
void getIDprocess(state_t *callerProcess, int parent);
void yield();


#endif