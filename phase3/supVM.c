#include "./headers/supVM.h"

// swap pool table
static swap_t swap_pool_table[POOLSIZE];
static int swap_pool_sem;

// page_table type per Uproc
pteEntry_t page_table[MAXPAGES];

void init_swap_pool_table()
{
    swap_pool_sem = 1;
    for (int i = 0; i < POOLSIZE; i++)
        swap_ptable[i].sw_asid = NOPROC;
}

void init_page_table(page_table pt, int asid)
{

    int npage = MAXPAGES - 1;

    for (int i = 0; i < npage; i++)
    {
        pt[i].pte_entry_hi = KUSEG + (i << VPNSHIFT) + (asid << ASIDSHIFT);
        pt[i].pte_entry_lo = DIRTYON;
    }

    pt[npage].pte_entry_hi = (memaddr *)0xBFFFF000 - USERSTACKTOP;
    pt[npage].pte_entry_lo = DIRTYON;

    return true;
}
