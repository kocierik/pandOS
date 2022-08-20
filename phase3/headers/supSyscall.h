#ifndef SYSCALL_H_INCLUDED3
#define SYSCALL_H_INCLUDED3

#include "../../generic_headers/pandos_types.h"
#include "supSyscall.h"
#include "supVM.h"
#include "TLBhandler.h"

void get_tod(support_t *s);
void terminate(support_t *s);
void write_to_printer(support_t *s);
void write_to_terminal(support_t *s);
void syscall_write(support_t *s, int IL_X);
void read_from_terminal(support_t *s);

#endif