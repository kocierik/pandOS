#include "headers/TLBhandler.h"

extern pcb_PTR currentActiveProc;
extern void klog_print(char *s);

// just a terminate wrapper
void trap()
{
    myprint("trap\n");
    SYSCALL(TERMINATE, 0, 0, 0);
}

/**
 * It takes the virtual address of the page fault, finds the corresponding page table entry, and writes
 * it to the TLB
 */
void uTLB_RefillHandler()
{
    myprint("utlb refill start\n");

    state_t *s = (state_t *)BIOSDATAPAGE;
    int index = ENTRYHI_GET_VPN(s->entry_hi);
    klog_print_dec(index);
    if (index == 0x3FFFF)
    {
        myprint("stack index \n");
        index = 31; /* stack */
    }
    else if (index < 0 || index > 31 || index == 31)
    { // da togliere
        myprint("index strano \n");
    }
    // serve fare un controllo sull'index???
    pteEntry_t pte = currentActiveProc->p_supportStruct->sup_privatePgTbl[index];
    setENTRYHI(pte.pte_entryHI);
    setENTRYLO(pte.pte_entryLO);
    TLBWR();

    myprint("utlb refill end\n");

    LDST(s);
}

/**
 * It handles general exceptions
 */
void general_execption_handler()
{
    myprint("gen exc\n");

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

/**
 * It handles the syscall exception by calling the appropriate function based on the value of the
 * syscall number in register a0
 *
 * @param exc_sd the support_t struct that contains the state of the exception
 */
void sup_syscall_handler(support_t *exc_sd)
{
    myprint("sysuser\n");

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