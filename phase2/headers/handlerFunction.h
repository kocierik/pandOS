#ifndef HANDFUNC_H_INCLUDED
#define HANDFUNC_H_INCLUDED

#include <umps/libumps.h>
#include <umps/arch.h>
#include "syscall.h"
#include "exceptionHandler.h"


int getBlockedSem(int bitAddress);
void pltTimerHandler(state_t *excState);
void intervallTimerHandler(state_t *excState);
void deviceIntHandler(int cause, state_t *excState);
void terminalHandler();

void passOrDie(int pageFault, state_t *excState);
void syscall_handler(state_t *callerProc);

#endif