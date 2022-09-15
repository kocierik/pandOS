#include "headers/exceptionHandler.h"
#include "../phase3/headers/p3test.h"

void exception_handler()
{
    state_t *exceptionState = (state_t *)BIOSDATAPAGE;
    int causeCode = CAUSE_GET_EXCCODE(getCAUSE());

<<<<<<< HEAD
    switch (causeCode)
    {
    case IOINTERRUPTS: // Interrupt
        interrupt_handler(exceptionState);
        break;
    case 1 ... 3: // TLB Exception
        tlb_handler(exceptionState);
        break;
    case 4 ... 7: // Trap
    case 9 ... 12:
        klog_print_dec(causeCode);
        trap_handler(exceptionState);
        break;
    case 8: // System Call
        syscall_handler(exceptionState);
        break;
    default:
        PANIC();
    }
}

// case 0 exception_handler()
void interrupt_handler(state_t *excState)
{
    myprint("int hand  ");
=======
    switch(causeCode){
        case IOINTERRUPTS:                      // Interrupt
            interrupt_handler(exceptionState);
            break;
        case 1 ... 3:                           // TLB Exception
            tlb_handler(exceptionState);
            break;
        case 4 ... 7:                           // Trap 
        case 9 ... 12:
            trap_handler(exceptionState);
            break;
        case 8:                                 // System Call
            syscall_handler(exceptionState);
            break;
        default:
            PANIC();
    }
}

// case 0 exception_handler() 
void interrupt_handler(state_t *excState) {
    myprint("int hand\n");
>>>>>>> parent of 33564e8 (aaaaaaaaaaaaaaaaaa)

    int cause = getCAUSE(); // Ritorna il registro CAUSE (3.3 pops)

    if CAUSE_IP_GET (cause, IL_CPUTIMER)
        plt_time_handler(excState);
    else if CAUSE_IP_GET (cause, IL_TIMER)
        intervall_timer_handler(excState);
    else if CAUSE_IP_GET (cause, IL_DISK)
        device_handler(IL_DISK, excState);
    else if CAUSE_IP_GET (cause, IL_FLASH)
        device_handler(IL_FLASH, excState);
    else if CAUSE_IP_GET (cause, IL_ETHERNET)
        device_handler(IL_ETHERNET, excState);
    else if CAUSE_IP_GET (cause, IL_PRINTER)
        device_handler(IL_PRINTER, excState);
    else if CAUSE_IP_GET (cause, IL_TERMINAL)
        device_handler(IL_TERMINAL, excState);

    PANIC();
}

// case 1 ... 3 exception_handler()
<<<<<<< HEAD
void tlb_handler(state_t *excState)
{
    myprint("tlb exc  ");
=======
void tlb_handler(state_t *excState) {
    myprint("case 1 ... 3 exception_handler()\n");
>>>>>>> parent of 33564e8 (aaaaaaaaaaaaaaaaaa)
    pass_up_or_die(PGFAULTEXCEPT, excState);
}

// case 4 ... 7 | 9 ... 12 exception_handler()
<<<<<<< HEAD
void trap_handler(state_t *excState)
{
    myprint("trap kern  ");
=======
void trap_handler(state_t *excState) {
    myprint("case 4-7 9-12 exception_handler()");
>>>>>>> parent of 33564e8 (aaaaaaaaaaaaaaaaaa)
    pass_up_or_die(GENERALEXCEPT, excState);
}

// case 8 case exception_handler()
void syscall_handler(state_t *callerProcState)
{
    int syscode = (*callerProcState).reg_a0;
    callerProcState->pc_epc += WORDLEN;

    if (((callerProcState->status << 28) >> 31))
    {
        callerProcState->cause |= (EXC_RI << CAUSESHIFT);
        pass_up_or_die(GENERALEXCEPT, callerProcState);
    }
    else
    {
        switch (syscode)
        {
        case CREATEPROCESS:
            create_process(callerProcState);
            break;
        case TERMPROCESS:
            terminate_process(callerProcState);
            break;
        case PASSEREN:
            passeren(callerProcState);
            break;
        case VERHOGEN:
            verhogen(callerProcState);
            break;
        case DOIO:
            do_IO_device(callerProcState);
            break;
        case GETTIME:
            get_cpu_time(callerProcState);
            break;
        case CLOCKWAIT:
            wait_for_clock(callerProcState);
            break;
        case GETSUPPORTPTR:
            get_support_data(callerProcState);
            break;
        case GETPROCESSID:
            get_ID_process(callerProcState);
            break;
        case YIELD:
            yield(callerProcState);
            break;
        default:
            trap_handler(callerProcState);
            break;
        }
    }
    load_or_scheduler(callerProcState);
}