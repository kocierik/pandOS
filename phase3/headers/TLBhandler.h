#ifndef TLBHANDLER_H_INCLUDED
#define TLBHANDLER_H_INCLUDED

#include "../../generic_headers/pandos_const.h"
#include "../../generic_headers/pandos_types.h"
#include <umps/libumps.h>
#include <umps/arch.h>
#include "umps/cp0.h"
#include "supSyscall.h"

void general_execption_hendler();
void sup_syscall_handler();

#endif