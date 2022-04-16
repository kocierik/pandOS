#include "headers/exceptionHandler.h"
#include "klog.c"


void exception_handler() {
    state_t *exceptionState = (state_t *)BIOSDATAPAGE;
    int causeCode = CAUSE_GET_EXCCODE(getCAUSE());

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
            PANIC();                            // Not expected situation
    }
}

/* IO_INTERRUPT CASE */
void interrupt_handler(state_t *excState) {
    int cause = getCAUSE(); // Get CAUSE register

    if      CAUSE_IP_GET(cause, IL_IPI)         ; /* To ignore, nothing to do */
    else if CAUSE_IP_GET(cause, IL_CPUTIMER)    plt_time_handler(excState);
    else if CAUSE_IP_GET(cause, IL_TIMER)       intervall_timer_handler(excState);
    else if CAUSE_IP_GET(cause, IL_DISK)        device_handler(IL_DISK, excState);
    else if CAUSE_IP_GET(cause, IL_FLASH)       device_handler(IL_FLASH, excState);
    else if CAUSE_IP_GET(cause, IL_ETHERNET)    device_handler(IL_ETHERNET, excState);
    else if CAUSE_IP_GET(cause, IL_PRINTER)     device_handler(IL_PRINTER, excState);
    else if CAUSE_IP_GET(cause, IL_TERMINAL)    terminal_handler();

    PANIC();
}

/* TLB Exception case */
void tlb_handler(state_t *excState) {
    pass_up_or_die(PGFAULTEXCEPT, excState);
}

/* TRAP case */
void trap_handler(state_t *excState) {
    pass_up_or_die(GENERALEXCEPT, excState);
}

/* SYSCALL case */
void syscall_handler(state_t *callerProcState) {
    int syscode = (*callerProcState).reg_a0;
    callerProcState->pc_epc += WORDLEN;
    
    if(((callerProcState->status << 28) >> 31)){
        callerProcState->cause |= (EXC_RI << CAUSESHIFT);
        pass_up_or_die(GENERALEXCEPT, callerProcState);
    } else {
        switch(syscode) {
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