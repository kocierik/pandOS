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
      return i;            // if i is not occupied, return i
  return (c++) % POOLSIZE; // implementazione fifo
}

/* Useful function for pager */

unsigned int backigStoreOperation(unsigned int asid, unsigned int frameStartAddress, unsigned int numDeviceBlock, char kindOfOperation)
{
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
/*
unsigned int writeBackingStore(unsigned int asid, unsigned int frameStartAddress, unsigned int numDeviceBlock){
  return backigStoreOperation(asid, frameStartAddress, numDeviceBlock, 'w');
}

unsigned int readBackingStore(unsigned int asid, unsigned int frameStartAddress, unsigned int numDeviceBlock){
  return backigStoreOperation(asid, frameStartAddress, numDeviceBlock, 'r');
}
*/
// page fault exception
// da completare

void pager()
{

  // Puntatore alla struttura di supporto del processo corrente
  support_t *supp = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
  state_t *suppState = &(supp->sup_exceptState[PGFAULTEXCEPT]);

  // Controllo la causa del TLB exception, se si tratta di un TLB-modification genero una trap
  if (suppState->cause == 1)
    trap();

  // Ottengo la mutua esclusione sulla Swap Pool table
  SYSCALL(PASSEREN, (int)&swap_pool_sem, 0, 0);

  // Page number mancante
  int index = ENTRYHI_GET_ASID(suppState->entry_hi);
  // int index = entryhi_to_index(suppState->entry_hi);

  // Frame dalla Swap Pool trovato utilizzando un algoritmo già implementato in Pandos
  int victim_page = pick_frame();

  unsigned int frameStartAddr = SWAPSTART + (victim_page * PAGESIZE);
  unsigned int missingPageNum = (supp->sup_exceptState[PGFAULTEXCEPT].entry_hi & GETPAGENO) >> VPNSHIFT;
  unsigned int asid = swap_pool_table[victim_page].sw_asid;

  // Controllo se il frame scelto è occupato
  if (!is_spframe_free(victim_page))
  {
    // Disabilito interrupt per eseguire le seguenti operazioni atomicamente
    setSTATUS(getSTATUS() & (DISABLEINTS));

    /*8.a*/
    /* marca come invalid la riga della tabella delle pagine corrispondente alla pagina vittima */
    pteEntry_t *victim_pte = swap_pool_table[victim_page].sw_pte;
    victim_pte->pte_entryLO = victim_pte->pte_entryLO & (!VALIDON); // TODO Controllare

    // 8.b
    // Aggiornamento del TLB
    setENTRYHI(victim_pte->pte_entryHI);
    TLBP();
    if ((getINDEX() & PRESENTFLAG) == 0)
    {
      setENTRYHI(victim_pte->pte_entryHI);
      setENTRYLO(victim_pte->pte_entryLO);
      TLBWI();
    }

    // Riattivo interrupt
    setSTATUS(getSTATUS() | IECON);

    /* WRITE FLASH */
    dtpreg_t *device = (dtpreg_t *)DEV_REG_ADDR(FLASHINT, asid - 1);
    device->data0 = (memaddr)frameStartAddr;
    const size_tt cmd = FLASHWRITE | index << 8;
    // TODO modifica commento -> 8 number of the least significant bit of BLOCKNUMBER in the COMMAND field.
    const int result = SYSCALL(DOIO, (int)&device->command, cmd, 0);
    // Controllo se ci sono errori, se si genero una trap.
    if (result != DEV_STATUS_READY)
      trap();
  }

  /* READ FLASH* /
  dtpreg_t *device = (dtpreg_t *)DEV_REG_ADDR(FLASHINT, asid - 1);
  device->data0 = (memaddr)frameStartAddr;
  const size_tt cmd = FLASHREAD | index << 8;
  const int result = SYSCALL(DOIO, (int)&device->command, cmd, 0);
  if (result != DEV_STATUS_READY)
    trap();

  // Update swap pool table entry
  unsigned int missingPageNum = (suppState->entry_hi & GETPAGENO) >> VPNSHIFT;

  swap_pool_table[supp->sup_asid].sw_asid = supp->sup_asid;
  swap_pool_table[supp->sup_asid].sw_pageNo = missingPageNum;
  swap_pool_table[supp->sup_asid].sw_pte = &(supp->sup_privatePgTbl[missingPageNum]);

  // Update page table
  setSTATUS(getSTATUS() & (DISABLEINTS));

  // sup_ptr->sup_privatePgTbl[missingPageNum].pte_entryLO = (frameStartAddr & 0xFFFFF000) | VALIDON | (sup_ptr->sup_privatePgTbl[missingPageNum].pte_entryLO & DIRTYON);

  // page_table[index].pte_entry_lo = frame_addr | VALIDON | DIRTYON;

  supp->sup_privatePgTbl[missingPageNum].pte_entryLO = (unsigned int)&swap_pool_table[victim_page] | VALIDON | DIRTYON;

  setSTATUS(getSTATUS() | IECON);

  // unsigned int deviceBlockNumber = swap_pool_table[victim_page].sw_pageNo; /* [0 - 31] */

  SYSCALL(VERHOGEN, (int)&swap_pool_sem, 0, 0);
  LDST(suppState);
}
