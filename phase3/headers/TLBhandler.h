#ifndef TLBHANDLER_H_INCLUDED
#define TLBHANDLER_H_INCLUDED

#include "../../generic_headers/pandos_const.h"
#include "../../generic_headers/pandos_types.h"
#include "../testers/h/tconst.h"
#include <umps/libumps.h>
#include <umps/arch.h>
#include "umps/cp0.h"
#include "supSyscall.h"
#include "supVM.h"
#include "TLBhandler.h"

void trap();
//int entryhi_to_index(memaddr enthi);
void uTLB_RefillHandler();
void general_execption_handler();
void sup_syscall_handler(support_t *exc_sd);

#endif