#ifndef HANDFUNC_H_INCLUDED
#define HANDFUNC_H_INCLUDED

#include <umps/libumps.h>
#include <umps/arch.h>
#include "syscall.h"
#include "exceptionHandler.h"


int getBlockedSem(int bitAddress);
void loadState(state_t *s);

/* Interrupt handler functions */
void plt_time_handler(state_t *excState);
void intervall_timer_handler(state_t *excState);
void device_handler(int cause, state_t *excState);
void terminal_handler();

/* Other excpetion function */
void pass_up_or_die(int pageFault, state_t *excState);
void syscall_handler(state_t *callerProc);

#endif