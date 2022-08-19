#include "headers/TLBhandler.h"

extern pcb_PTR currentActiveProc;

void uTLB_RefillHandler()
{
    state_t *s = (state_t *)BIOSDATAPAGE;
    // get index from entry hi
    int index = 0; // da modificare
    pteEntry_t pte = currentActiveProc->p_support->sup_privatePgTbl[index];
    setENTRYHI(pte.pte_entry_hi);
    setENTRYLO(pte.pte_entry_lo);
    TLBWR();
    LDST(s);
}

void general_execption_hendler()
{
    support_t *exc_sd = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    state_t *save = &exc_sd->sup_except_state[GENERALEXCEPT];
    save->pc_epc += WORD_SIZE; // da controllare

    switch (CAUSE_GET_EXCCODE(exc_sd->sup_except_state[GENERALEXCEPT].cause))
    {
    case 8:
        sup_syscall_handler(exc_sd);
        break;
    default:
        SYSCALL(TERMINATE, 0, 0, 0); // trap
    }
    LDST(save);
}

void sup_syscall_handler(support_t *exc_sd)
{
    int ret;

    switch (exc_sd->sup_except_state[GENERALEXCEPT].reg_a0)
    {
    case GET_TOD:
        ret = get_tod();
        break;
    case WRITEPRINTER:
        ret = write_printer();
        break;
    case WRITETERMINAL:
        ret = write_terminal();
        break;
    case READTERMINAL:
        ret = read_terminal();
        break;
    case TERMINATE:
        terminate(exc_sd);
        break;
    default:
        PANIC();
    }
    // currentActiveProc->pc_epc += WORDLEN; // da controllare: già fatto in general exc handl
    exc_sd->sup_except_state[GENERALEXCEPT].reg_v0 = ret;
}