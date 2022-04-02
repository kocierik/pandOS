#ifndef SYSCALL_H_INCLUDED
#define SYSCALL_H_INCLUDED

#include "../../phase1/headers/pcb.h"
#include "../../phase1/headers/asl.h"
#include "../../phase1/headers/listx.h"


void createProcess();
void terminateProcess();
void passeren();
void verhogen();
void doIOdevice();
cpu_t getCpuTime();
void waitForClock();
support_t* getSupportData();
int getIDprocess();
void yield();


#endif