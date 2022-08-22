#include "headers/TLBhandler.h"

extern pcb_PTR currentActiveProc;

// convert the entryhi value into an index
int entryhi_to_index(memaddr entry_hi)
{
    return ((entry_hi & 0xFFFFF000) >> VPNSHIFT) - 0x80000; // da controllare
}

// tlb refill handler
void uTLB_RefillHandler()
{
    state_t *s = (state_t *)BIOSDATAPAGE;
    int index = entryhi_to_index(s->entry_hi);
    pteEntry_t pte = currentActiveProc->p_supportStruct->sup_privatePgTbl[index];
    setENTRYHI(pte.pte_entryHI);
    setENTRYLO(pte.pte_entryLO);
    TLBWR();
    LDST(s);
}

void general_execption_handler()
{
    support_t *exc_sd = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    state_t *save = &exc_sd->sup_exceptState[GENERALEXCEPT];
    save->pc_epc += WORD_SIZE;

    switch (CAUSE_GET_EXCCODE(exc_sd->sup_exceptState[GENERALEXCEPT].cause))
    {
    case SYSEXCEPTION:
        sup_syscall_handler(exc_sd);
        break;
    default:
        SYSCALL(TERMINATE, 0, 0, 0); // trap
    }

    LDST(save);
}

void sup_syscall_handler(support_t *exc_sd)
{
    switch (exc_sd->sup_exceptState[GENERALEXCEPT].reg_a0)
    {
    case TERMINATE:
        terminate(exc_sd);
        break;
    case GET_TOD:
        get_tod(exc_sd);
        break;
    case WRITEPRINTER:
        write_to_printer(exc_sd);
        break;
    case WRITETERMINAL:
        write_to_terminal(exc_sd);
        break;
    case READTERMINAL:
        read_from_terminal(exc_sd);
        break;
    default:
        SYSCALL(TERMINATE, 0, 0, 0); // trap
    }
}