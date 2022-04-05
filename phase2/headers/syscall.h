#ifndef SYSCALL_H_INCLUDED
#define SYSCALL_H_INCLUDED

#include "../../generic_headers/pandos_const.h"
#include "../../phase1/headers/pcb.h"
#include "../../phase1/headers/asl.h"
#include "../../phase1/headers/listx.h"
#include "umps/const.h"
#include "umps/libumps.h"


/* Funzioni di aiuto */
void __terminate_process(pcb_PTR p);
void term_proc_and_child(pcb_PTR rootPtr);
int lenQ(struct list_head queue);
//pcb_PTR findPcb(int pid, struct list_head queue);


void createProcess(state_t * callerProcess);
void terminateProcess(int *pid, pcb_PTR callerProcess);
void passeren(int *semaddr);
void verhogen(int *semaddr);
void doIOdevice(int *cmdAddr, int cmdValue);
void getCpuTime();
void waitForClock();
support_t* getSupportData();
int getIDprocess(int parent);
void yield();


#endif