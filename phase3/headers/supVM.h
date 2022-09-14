#ifndef VMSUPPORT_H_INCLUDED
#define VMSUPPORT_H_INCLUDED

#include "../../generic_headers/pandos_const.h"
#include "../../generic_headers/pandos_types.h"
#include "supSyscall.h"
#include "p3test.h"
#include "TLBhandler.h"
#include <umps/cp0.h>

#define SWAP_POOL_ADDR (memaddr)0x20020000

// swap pool table
static swap_t swap_pool_table[POOLSIZE];
static int swap_pool_sem;

// page table entry per Uproc
typedef pteEntry_t page_table[MAXPAGES];


void bp();


void init_swap_pool_table();
void init_page_table(pteEntry_t pt[MAXPAGES], int asid);
int pick_frame();
int is_spframe_free(int i);
void off_interrupts();
void on_interrupts();
int read_flash(int asid, int block, void *dest);
int write_flash(int asid, int block, void *src);
void update_tlb(pteEntry_t p);
unsigned int backing_store_op(int asid, memaddr addr, unsigned int num_dev_block, char mode);
unsigned int write_backing_store(int asid, memaddr addr, unsigned int num_dev_block);
unsigned int read_backing_store(int asid, memaddr addr, unsigned int num_dev_block);
void pager();

#endif