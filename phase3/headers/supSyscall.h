#ifndef SYSCALL_H_INCLUDED3
#define SYSCALL_H_INCLUDED3

#include "../../generic_headers/pandos_const.h"
#include "../../generic_headers/pandos_types.h"
#include <umps/libumps.h>
#include <umps/arch.h>
#include "umps/cp0.h"
#include "supSyscall.h"
#include "supVM.h"
#include "TLBhandler.h"

#define PRINTCHR 2
#define TERMSTATMASK 0xFF
#define DEV_STATUS_READY 1

void get_tod(support_t *s);
void terminate(support_t *s);
void write_to_printer(support_t *s);
void write_to_terminal(support_t *s);
void syscall_write(support_t *s, int IL_X);
void read_from_terminal(support_t *sup, char *virtualAddr);

#endif
