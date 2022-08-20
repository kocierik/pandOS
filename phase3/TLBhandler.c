#include "headers/TLBhandler.h"

extern pcb_PTR currentActiveProc;

int entryhi_to_index(memaddr enthi)
{
}

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

void general_execption_hendler()
{
    support_t *exc_sd = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    state_t *save = &exc_sd->sup_exceptState[GENERALEXCEPT];
    save->pc_epc += WORD_SIZE; // da controllare

    switch (CAUSE_GET_EXCCODE(exc_sd->sup_exceptState[GENERALEXCEPT].cause))
    {
    case 8:
        sup_syscall_handler(exc_sd);
        break;
    default:
        SYSCALL(TERMINATE, 0, 0, 0); // trap
    }

    // currentActiveProc->pc_epc += WORDLEN; // da controllare: già fatto?
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
        PANIC();
    }
}