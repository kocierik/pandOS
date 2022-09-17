#include "headers/TLBhandler.h"

// just a terminate wrapper
void trap()
{
    myprint(" usertrap ");
    SYSCALL(TERMINATE, 0, 0, 0);
}

/**
 * It takes the virtual address of the page fault, finds the corresponding page table entry, and writes
 * it to the TLB
 */
void uTLB_RefillHandler()
{
    state_t *s = (state_t *)BIOSDATAPAGE;
    int index = ENTRYHI_GET_VPN(s->entry_hi);

    if (index == 0x3FFFF)
        index = 31;
    else if (index < 0 || index > 31)
        klog_print("BELLA");

    pteEntry_t p = currentActiveProc->p_supportStruct->sup_privatePgTbl[index];
    setENTRYHI(p.pte_entryHI);
    setENTRYLO(p.pte_entryLO);
    TLBWR();

    klog_print("!");

    LDST(s);
}

/**
 * It handles general exceptions
 */
void general_execption_handler()
{
    myprint("GEN exc  ");

    support_t *exc_sd = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    state_t *save = &exc_sd->sup_exceptState[GENERALEXCEPT];

    int code = CAUSE_GET_EXCCODE(exc_sd->sup_exceptState[GENERALEXCEPT].cause);

    switch (code)
    {
    case SYSEXCEPTION:
        sup_syscall_handler(exc_sd);
        break;
    default:
        klog_print("trapGenEx   ");
        klog_print_dec(code);
        trap();
    }

    save->pc_epc += WORD_SIZE;
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
    myprint("sysuser ");

    // currentActiveProc->p_s.pc_epc += WORD_SIZE;
    int code = exc_sd->sup_exceptState[GENERALEXCEPT].reg_a0;

    switch (code)
    {
    case TERMINATE:
        terminate(exc_sd);
        break;
    case GET_TOD:
        get_tod(exc_sd);
        break;
    case WRITEPRINTER:
        myprint("WRITEPRINTER ");
        write_to_printer(exc_sd);
        break;
    case WRITETERMINAL:
        write_to_terminal(exc_sd);
        break;
    case READTERMINAL:
        read_from_terminal(exc_sd, (char *)exc_sd);
        break;
    default:
        klog_print("trapSupHan  ");
        klog_print_dec(code);
        trap();
    }
}