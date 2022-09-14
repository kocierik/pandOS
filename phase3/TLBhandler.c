#include "headers/TLBhandler.h"

extern pcb_PTR currentActiveProc;
extern void klog_print(char *s);

// just a terminate wrapper
void trap()
{
    klog_print("\ntrappolona\n");
    bp();
    SYSCALL(TERMINATE, 0, 0, 0);
}

void uTLB_RefillHandler()
{
    state_t *s = (state_t *)BIOSDATAPAGE;
    int index = ENTRYHI_GET_VPN(s->entry_hi);
    if (index == 0x3FFFF){ 
		index = 31; /* stack */
    } // serve fare un controllo sull'index???
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
        trap();
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
        read_from_terminal(exc_sd, (char *)exc_sd);
        break;
    default:
        trap();
    }
}