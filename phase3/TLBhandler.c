#include "headers/TLBhandler.h"

extern pcb_PTR currentActiveProc;

void general_execption_hendler()
{
    support_t *exc_sd = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    state_t *save = &exc_sd->sup_except_state[GENERALEXCEPT];
    switch (CAUSE_GET_EXCCODE(exc_sd->sup_except_state[GENERALEXCEPT].cause))
    {
    case 8:
        sup_syscall_handler(exc_sd);
        break;
    default:
        SYSCALL(TERMINATE, 0, 0, 0);
    }
    save->pc_epc += WORD_SIZE;
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
    default:
        terminate();
        break;
    }
    currentActiveProc->pc_epc += WORDLEN; // da controllare
    exc_sd->sup_except_state[GENERALEXCEPT].reg_v0 = ret;
}