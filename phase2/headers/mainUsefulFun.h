#ifndef USEFULFUNC_H_INCLUDED
#define USEFULFUNC_H_INCLUDED

#include "../../generic_headers/pandos_const.h"
#include "../../generic_headers/pandos_types.h"
#include "../../phase1/headers/asl.h"
#include "../../phase1/headers/pcb.h"
#include "../../phase1/headers/listx.h"
#include "globals.h"

/* Extern functions */
extern void test();
extern void scheduler();
extern void uTLB_RefillHandler();
extern void exception_handler();

void init_global_var();
void init_passupvector(passupvector_t *vector);
void insert_ready_queue(int priority, pcb_PTR process);

void copy_state(state_t *new, state_t *old);

#endif