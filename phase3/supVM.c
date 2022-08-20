#include "./headers/supVM.h"

// swap pool table
static swap_t swap_pool_table[POOLSIZE];
static int swap_pool_sem;

// page table entry per Uproc
pteEntry_t page_table[MAXPAGES];

void init_swap_pool_table()
{
    swap_pool_sem = 1;
    for (int i = 0; i < POOLSIZE; i++)
        swap_pool_table[i].sw_asid = NOPROC;
}

void init_page_table(pteEntry_t pt[MAXPAGES], int asid)
{

    int npage = MAXPAGES - 1;

    for (int i = 0; i < npage; i++)
    {
        pt[i].pte_entryHI = KUSEG + (i << VPNSHIFT) + (asid << ASIDSHIFT);
        pt[i].pte_entryLO = DIRTYON;
    }

    pt[npage].pte_entryHI = 0xBFFFF000 - USERSTACKTOP;
    pt[npage].pte_entryLO = DIRTYON;
}

// if i-th swap pool table frame is free return true, false otherwise
int is_spframe_free(int i)
{
    return swap_pool_table[i].sw_asid == NOPROC;
}

int pick_frame()
{
    static int c = 0;
    for (int i = 0; i < POOLSIZE; i++)
        if (is_spframe_free(i))
            return i;        // if i is not occupied, return i
    return (c++) % POOLSIZE; // implementazione fifo
}

void pager()
{
    support_t *supp = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    state_t *save = &supp->sup_exceptState[PGFAULTEXCEPT];

    if (save->cause == 1)            // TLB-Modification exception
        SYSCALL(TERMINATE, 0, 0, 0); // trap

    SYSCALL(PASSEREN, (int)&swap_pool_sem, 0, 0);
    int index = entryhi_to_index(save->entry_hi);
    int victim = pick_frame();

    if (!is_spframe_free(victim))
    {
        /* 8.
        if frame i is currently occupied, assume it is occupied by logical page
        number k belonging to process x (ASID) and that it is “dirty” (i.e.
        been modified):
        (a) Update process x’s Page Table: mark Page Table entry k as not
        valid. This entry is easily accessible, since the Swap Pool table’s
        entry i contains a pointer to this Page Table entry.
        (b) Update the TLB, if needed. The TLB is a cache of the most
        recently executed process’s Page Table entries. If process x’s page
        k’s Page Table entry is currently cached in the TLB it is clearly
        out of date; it was just updated in the previous step.
        Important Point: This step and the previous step must be ac-
        complished atomically. [Section 4.5.3]
        (c) Update process x’s backing store. Write the contents of frame i
        to the correct location on process x’s backing store/flash device.
        [Section 4.5.1]
        Treat any error status from the write operation as a program trap.
        [Section 4.8]*/
    }
    /*
    9. Read the contents of the Current Process’s backing store/flash device
    logical page p into frame i. [Section 4.5.1]
    Treat any error status from the read operation as a program trap.
    [Section 4.8]
    10. Update the Swap Pool table’s entry i to reflect frame i’s new contents:
    page p belonging to the Current Process’s ASID, and a pointer to the
    Current Process’s Page Table entry for page p.
    11. Update the Current Process’s Page Table entry for page p to indicate
    it is now present (V bit) and occupying frame i (PFN field).
    12. Update the TLB. The cached entry in the TLB for the Current Pro-
    cess’s page p is clearly out of date; it was just updated in the previous step.
    Important Point: This step and the previous step must be accom-
    plished atomically. [Section 4.5.3]
    */
    SYSCALL(VERHOGEN, (int)&swap_pool_sem, 0, 0);
    LDST(save);
}