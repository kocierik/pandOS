#ifndef SYSCALL_H_INCLUDED
#define SYSCALL_H_INCLUDED

#include "../../phase1/headers/pcb.h"


void createProcess();
void terminateProcess();
void passeren();
void verhogen();
void doIOdevice();
void getCpuTime();
void waitForClock();
void getSupportData();
void getIDprocess();
void yeild();


#endif