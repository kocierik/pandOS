#ifndef HANDFUNC_H_INCLUDED
#define HANDFUNC_H_INCLUDED

#include <umps/libumps.h>
#include <umps/arch.h>
#include "syscall.h"
#include "scheduler.h"
#include "exceptionHandler.h"
#include "globals.h"

/* Usefull external function */
extern void insert_ready_queue(int prio, pcb_PTR p);

/* Interrupt handler functions */
void plt_time_handler(state_t *excState);
void intervall_timer_handler(state_t *excState);
void device_handler(int cause, state_t *excState);

/* Other excpetion function */
void pass_up_or_die(int pageFault, state_t *excState);

#endif