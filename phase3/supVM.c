#include "./headers/supVM.h"

void bp() {}
extern void klog_print(char *s);
extern void klog_print_dec(unsigned int num);

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

// Disabilita gli interrupts
void off_interrupts()
{
  setSTATUS(getSTATUS() & (DISABLEINTS));
}

// Abilita gli interrupts
void on_interrupts()
{
  setSTATUS(getSTATUS() & (IECON));
}

// Legge dal dispositivo di flash
int read_flash(int asid, int block, void *dest)
{

  off_interrupts();
  dtpreg_t *dev = (dtpreg_t *)DEV_REG_ADDR(FLASHINT, asid - 1);
  dev->data0 = (memaddr)dest;
  int cmd = FLASHREAD | block << 8;
  int res = SYSCALL(DOIO, (int)&dev->command, cmd, 0);
  klog_print("ciao");
  bp();
  on_interrupts();
  return res;
}

// Scrive su un disposivo di flash
int write_flash(int asid, int block, void *src)
{
  off_interrupts();
  dtpreg_t *dev = (dtpreg_t *)DEV_REG_ADDR(FLASHINT, asid - 1);
  dev->data0 = (memaddr)src;
  int cmd = FLASHWRITE | block << 8;
  int ret = SYSCALL(DOIO, (int)&dev->command, cmd, 0);
  on_interrupts();
  return ret;
}

// aggiorna TLB
void update_tlb(pteEntry_t p)
{
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
void pager()
{
  // Puntatore alla struttura di supporto del processo corrente
  support_t *supp = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
  state_t *supp_state = &(supp->sup_exceptState[PGFAULTEXCEPT]);

  // Controllo la causa del TLB exception, se si tratta di un TLB-modification genero una trap
  if (supp_state->cause == 1)
    trap();

  // Ottengo la mutua esclusione sulla Swap Pool table
  SYSCALL(PASSEREN, (int)&swap_pool_sem, 0, 0);

  // Prendo una pagina vittima da sostituire
  int victim_page = pick_frame();

  swap_t swap_entry = swap_pool_table[victim_page];
  int asid = swap_entry.sw_asid;

  // Page number mancante
  int index = swap_entry.sw_pageNo;

  // Dati utili
  memaddr victim_page_addr = SWAP_POOL_ADDR + (victim_page * PAGESIZE);
  memaddr vpn = ENTRYHI_GET_VPN(supp_state->entry_hi);

  // Controllo se il frame scelto è occupato
  if (!is_spframe_free(victim_page))
  {
    off_interrupts();

    /*8.a Marca come invalid la riga della tabella delle pagine corrispondente alla pagina vittima */
    pteEntry_t *victim_pte = swap_entry.sw_pte;
    victim_pte->pte_entryLO &= !VALIDON;

    // 8.b Aggiornamento del TLB
    update_tlb(*victim_pte);

    on_interrupts();

    /* WRITE FLASH */
    if (write_flash(asid, index, (void *)victim_page_addr) != DEV_STATUS_READY)
      trap();
  }

  /* READ FLASH */
  if (read_flash(asid, index, (void *)victim_page_addr) != DEV_STATUS_READY)
    trap();

  off_interrupts();

  // Adding entry to swap table
  swap_entry.sw_asid = supp->sup_asid;
  swap_entry.sw_pageNo = vpn;
  swap_entry.sw_pte = &(supp->sup_privatePgTbl[vpn]);

  // Update page table
  supp->sup_privatePgTbl[vpn].pte_entryLO = victim_page_addr | VALIDON | DIRTYON;

  // Update TLB
  update_tlb(supp->sup_privatePgTbl[index]); // da controllare

  on_interrupts();

  SYSCALL(VERHOGEN, (int)&swap_pool_sem, 0, 0);
  LDST(supp_state);
}