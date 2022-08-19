#ifndef VMSUPPORT_H_INCLUDED
#define VMSUPPORT_H_INCLUDED

#include "../../generic_headers/pandos_const.h"
#include "../../generic_headers/pandos_types.h"
#include "supSyscall.h"
#include "TLBhandler.h"

#define SWAP_POOL_ADDR (memaddr *)0x20020000

// page_table type per Uproc
typedef pteEntry_t page_table[MAXPAGES];

void init_swap_pool_table();
void init_page_table(page_table pt, int asid);

#endif