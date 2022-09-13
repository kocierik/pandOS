#ifndef VMSUPPORT_H_INCLUDED
#define VMSUPPORT_H_INCLUDED

#include "../../generic_headers/pandos_const.h"
#include "../../generic_headers/pandos_types.h"
#include "supSyscall.h"
#include "p3test.h"
#include "TLBhandler.h"
#include <umps3/umps/cp0.h>

#define SWAP_POOL_ADDR (memaddr *)0x20020000

// swap pool table
static swap_t swap_pool_table[POOLSIZE];
static int swap_pool_sem;

// page table entry per Uproc
typedef pteEntry_t page_table[MAXPAGES];

void init_swap_pool_table();
void init_page_table(pteEntry_t pt[MAXPAGES], int asid);
int pick_frame();
int is_spframe_free(int i);
unsigned int backigStoreOperation(unsigned int asid, unsigned int frameStartAddress, unsigned int numDeviceBlock, char kindOfOperation);
unsigned int writeBackingStore(unsigned int asid, unsigned int frameStartAddress, unsigned int numDeviceBlock);
unsigned int readBackingStore(unsigned int asid, unsigned int frameStartAddress, unsigned int numDeviceBlock);
void pager();

#endif