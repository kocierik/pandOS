#ifndef SYSCALL_H_INCLUDED
#define SYSCALL_H_INCLUDED

#include "../../generic_headers/pandos_const.h"
#include "../../phase1/headers/pcb.h"
#include "../../phase1/headers/asl.h"
#include "../../phase1/headers/listx.h"
#include "umps/const.h"
#include "umps/libumps.h"


/* Funzioni di aiuto */
void __terminateProcess(pcb_PTR p);
void terminateDescendance(pcb_PTR rootPtr);
pcb_PTR findPcb(int pid, struct list_head queue);


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