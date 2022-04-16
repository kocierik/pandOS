#ifndef SYSCALL_H_INCLUDED
#define SYSCALL_H_INCLUDED
#include "syscallUsefulFun.h"

/* Exsternal function */
extern void insert_ready_queue(int prio, pcb_PTR p);

void create_process(state_t *excState);
void terminate_process(state_t *excState);
void passeren(state_t *excState);
pcb_PTR P(int *semaddr, state_t *excState);
void verhogen(state_t *excState);
pcb_PTR V(int *semaddr, state_t *excState);
void do_IO_device(state_t *excState);
void get_cpu_time(state_t *excState);
void wait_for_clock(state_t *excState);
void get_support_data(state_t *excState);
void get_ID_process(state_t *excState);
void yield(state_t *excState);


#endif