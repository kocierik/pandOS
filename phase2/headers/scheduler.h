#ifndef SCHEDULER_H_INCLUDED
#define SCHEDULER_H_INCLUDED

#include "../../phase1/headers/pcb.h"
#include "../../generic_headers/pandos_const.h"
#include "umps/libumps.h"
#include "syscall.h"

void scheduler();
void scheduler_empty_queues();

#endif