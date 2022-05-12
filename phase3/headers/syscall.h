#ifndef SYSCALL_H_INCLUDED3
#define SYSCALL_H_INCLUDED3

#include "../../generic_headers/pandos_types.h"

void get_tod(state_t *excState);
void terminate(state_t *excState);
void write_to_printer(state_t *excState);
void write_to_terminal(state_t *excState);
void read_from_terminal(state_t *excState);

#endif