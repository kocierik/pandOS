#ifndef VMSUPPORT_H_INCLUDED
#define VMSUPPORT_H_INCLUDED

#include "../../generic_headers/pandos_const.h"
#include "../../generic_headers/pandos_types.h"
#include "supSyscall.h"
#include "p3test.h"
#include "TLBhandler.h"
#include <umps/cp0.h>

#define SWAP_POOL_ADDR (memaddr)0x20020000

void bp();

void init_swap_pool_table();
void init_page_table(pteEntry_t pt[MAXPAGES], int asid);
int pick_frame();
int is_spframe_free(int i);
void off_interrupts();
void on_interrupts();
int flash(int asid, int block, memaddr addr, char mode);
void update_tlb(pteEntry_t p);
void pager();

#endif