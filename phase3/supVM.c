#include "./headers/supVM.h"

void bp() {}

/**
 * Initialize the swap pool table and its semaphore.
 */
void init_swap_pool_table()
{
    swap_pool_sem = 1;
    for (int i = 0; i < POOLSIZE; i++)
        swap_pool_table[i].sw_asid = NOPROC;
}

/**
 * It initializes the page table for the process
 *
 * @param pt the page table
 * @param asid the address space identifier
 */
void init_page_table(pteEntry_t pt[MAXPAGES], int asid)
{
    for (int i = 0; i < MAXPAGES - 1; i++)
    {
        pt[i].pte_entryHI = KUSEG + (i << VPNSHIFT) + (asid << ASIDSHIFT);
        pt[i].pte_entryLO = DIRTYON;
    }
    // stack
    pt[MAXPAGES - 1].pte_entryHI = 0xBFFFF000 + (asid << ASIDSHIFT);
    pt[MAXPAGES - 1].pte_entryLO = DIRTYON;
}

// if i-th swap pool table frame is free return true, false otherwise
int is_spframe_free(int i)
{
    return swap_pool_table[i].sw_asid == NOPROC;
}

/**
 * It returns the first free frame in the pool, or the next frame in the pool in fifo order
 *
 * @return The index of the victim frame
 */
int pick_frame()
{
    static int c = 0;
    for (int i = 0; i < POOLSIZE; i++)
        if (is_spframe_free(i))
            return i;        // if i is not occupied, return i
    return (c++) % POOLSIZE; // implementazione fifo
}

// Disabilita gli interrupts
void off_interrupts()
{
    setSTATUS(getSTATUS() & !IECON);
}

// Abilita gli interrupts
void on_interrupts()
{
    setSTATUS(getSTATUS() | IECON);
}

/**
 * It writes the address of the flash memory to be accessed in the data0 register of the device, then
 * it issues a DOIO command to the device, with the command field set to either FLASHWRITE or
 * FLASHREAD, depending on the mode parameter
 *
 * @param asid the ASID of the device (1-8)
 * @param block the block number to read/write
 * @param addr the address of the data to be written or read
 * @param mode 'r' for read, 'w' for write
 *
 * @return The result of the syscall.
 */
int flash(int asid, int block, memaddr addr, char mode)
{
    // da aggiungere i semafori
    off_interrupts();
    //myprint("flash  ");
    dtpreg_t *dev = (dtpreg_t *)DEV_REG_ADDR(FLASHINT, asid - 1);
    dev->data0 = addr;
    int cmd = (mode == 'w') ? FLASHWRITE : FLASHREAD | block << 8;
    int res = SYSCALL(DOIO, (int)&dev->command, cmd, 0);
    on_interrupts();
    return res;
}

/**
 * It updates the TLB with the given page table entry.
 *
 * @param p the page table entry
 */
void update_tlb(pteEntry_t p)
{
    //myprint("tlb update  ");
    setENTRYHI(p.pte_entryHI);
    TLBP();
    if ((getINDEX() & PRESENTFLAG) == 0)
    {
        setENTRYHI(p.pte_entryHI);
        setENTRYLO(p.pte_entryLO);
        TLBWI();
    }
}

// da completare
/**
 * It reads the page from the flash memory and puts it in the swap pool
 */
void pager()
{
    //myprint("pager start  ");

    support_t *supp = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    state_t *supp_state = &(supp->sup_exceptState[PGFAULTEXCEPT]);

    if (supp_state->cause == 1) // if cause is TLB-modification, then trap
        trap();

    SYSCALL(PASSEREN, (int)&swap_pool_sem, 0, 0);

    // Prendo una pagina vittima da sostituire
    int victim_page = pick_frame();
    swap_t swap_entry = swap_pool_table[victim_page];
    memaddr victim_page_addr = SWAP_POOL_ADDR + (victim_page * PAGESIZE);
    int vpn = ENTRYHI_GET_VPN(supp_state->entry_hi);
    vpn = (vpn < 0 || vpn > 31) ? 31 : vpn; // controllo di sicurezza

    // Controllo se il frame scelto è occupato
    if (!is_spframe_free(victim_page))
    {
        off_interrupts();

        /*8.a Marca come invalid la riga della tabella delle pagine corrispondente alla pagina vittima */
        pteEntry_t *victim_pte = swap_entry.sw_pte;
        victim_pte->pte_entryLO &= !VALIDON;

        update_tlb(*victim_pte);

        on_interrupts();

        /* WRITE FLASH */
        if (flash(swap_entry.sw_asid, swap_entry.sw_pageNo, victim_page_addr, 'w') != READY)
            trap();
    }

    /* READ FLASH */
    if (flash(supp->sup_asid, vpn, victim_page_addr, 'r') != READY)
        trap();

    off_interrupts();

    // Adding entry to swap table
    swap_entry.sw_asid = supp->sup_asid;
    swap_entry.sw_pageNo = vpn;
    swap_entry.sw_pte = &(supp->sup_privatePgTbl[vpn]);

    // Update page table
    supp->sup_privatePgTbl[vpn].pte_entryLO = victim_page_addr | VALIDON | DIRTYON;

    // Update TLB
    update_tlb(supp->sup_privatePgTbl[swap_entry.sw_pageNo]); // da controllare TBCLR()

    on_interrupts();

    SYSCALL(VERHOGEN, (int)&swap_pool_sem, 0, 0);

    myprint("pager end\n");

    LDST(supp_state);
}