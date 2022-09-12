#include "./headers/supVM.h"
#define SWAPSTART 0x20020000 

extern int semaphore[NSUPPSEM];

// init semaphore and swap pool table
void init_swap_pool_table()
{
  swap_pool_sem = 1;
  for (int i = 0; i < POOLSIZE; i++)
    swap_pool_table[i].sw_asid = NOPROC;
}

// init a page table with the right asid and addresses
void init_page_table(page_table pt, int asid)
{
  for (int i = 0; i < MAXPAGES - 1; i++)
    {
      pt[i].pte_entryHI = KUSEG + (i << VPNSHIFT) + (asid << ASIDSHIFT);
      pt[i].pte_entryLO = DIRTYON;
    }
  // stack
  pt[MAXPAGES - 1].pte_entryHI = 0xBFFFF000 + (asid << ASIDSHIFT); //- USERSTACKTOP;
  pt[MAXPAGES - 1].pte_entryLO = DIRTYON;
}

// if i-th swap pool table frame is free return true, false otherwise
int is_spframe_free(int i)
{
  return swap_pool_table[i].sw_asid == NOPROC;
}

// pick an index (fifo implemented)
int pick_frame()
{
  static int c = 0;
  for (int i = 0; i < POOLSIZE; i++)
    if (is_spframe_free(i))
      return i;        // if i is not occupied, return i
  return (c++) % POOLSIZE; // implementazione fifo
}



// Funzioni supporto per pager

unsigned int backigStoreOperation(unsigned int asid, unsigned int frameStartAddress, unsigned int numDeviceBlock, char kindOfOperation){
  /*
  int index = 8 + asid-1;
  SYSCALL(PASSEREN, &semaphore[index], 0, 0);

  devreg_t* devReg = (devreg_t*) DEV_REG_ADDR(FLASHINT, asid-1);
  devReg->dtp.data0 = frameStartAddress;

  unsigned int command;
  // Verifico che sia un operazione di lettura (reading) o scrittura (writing)
  if (kindOfOperation == 'r') command = (numDeviceBlock << 8) | FLASHREAD;
  else command = (numDeviceBlock << 8) | FLASHWRITE;

  int status = SYSCALL(DOIO, (unsigned int) &(devReg->dtp.command), command, 0);
  SYSCALL(VERHOGEN, &semaphore[index], 0, 0);

  return status;
  */
}

unsigned int writeBackingStore(unsigned int asid, unsigned int frameStartAddress, unsigned int numDeviceBlock){
  return backigStoreOperation(asid, frameStartAddress, numDeviceBlock, 'w');
}

unsigned int readBackingStore(unsigned int asid, unsigned int frameStartAddress, unsigned int numDeviceBlock){
  return backigStoreOperation(asid, frameStartAddress, numDeviceBlock, 'r');
}

// page fault exception
// da completare
void pager()
{

  // Puntatore alla struttura di supporto del processo corrente
  support_t *supp = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
  state_t *save = &supp->sup_exceptState[PGFAULTEXCEPT];

  // Controllo la causa del TLB exception, se si tratta di un TLB-modification genero una trap
  if (save->cause == 1)
    SYSCALL(TERMINATE, 0, 0, 0); // Trap

  // Ottengo la mutua esclusione sulla Swap Pool table
  SYSCALL(PASSEREN, (int)&swap_pool_sem, 0, 0);

  // Trovo il page number mancante
  int index = entryhi_to_index(save->entry_hi);

  // Prendo una frame dalla Swap Pool utilizzando un algoritmo già implementato in Pandos
  int victim_page = pick_frame();

  unsigned int frameStartAddr = SWAPSTART + (victim_page * PAGESIZE);  
	unsigned int missingPageNum = (supp->sup_exceptState[PGFAULTEXCEPT].entry_hi & GETPAGENO) >> VPNSHIFT;

  // Controllo se il frame scelto è occupato
  if (!is_spframe_free(victim_page))
  {
    //TODO vedi se dovresti disattivare interrupt
    /* marca come invalid la riga della tabella delle pagine corrispondente alla pagina vittima */
    pteEntry_t *victim_pte = swap_pool_table[victim_page].sw_pte; 
    //victim_pte->pte_entryLO = victim_pte->pte_entryLO & (VALIDON) //TODO controlla se può servire

    
    // Aggiornamento del TLB
    setENTRYHI(victim_pte->pte_entryHI);                                                                                       
    TLBP();
    if ((getINDEX() & 0x80000000) == 0) {                                                                                      
      setENTRYHI(victim_pte->pte_entryHI);                                                                                     
      setENTRYLO(victim_pte->pte_entryLO);                                                                                     
      TLBWI();                                                                                                                 
    }             
    //TODO vedi se dovresti riattivare interrupt
    setSTATUS(getSTATUS() | IECON);                                                                                            

    unsigned int asid = swap_pool_table[victim_page].sw_asid;                                                                 
    unsigned int deviceBlockNumber = swap_pool_table[victim_page].sw_pageNo; /* [0 - 31] */                                   
    
    

  }

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
