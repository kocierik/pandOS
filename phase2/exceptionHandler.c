#include "headers/exceptionHandler.h"
#include "klog.c"

extern pcb_PTR currentActiveProc;


void exception_handler() {
    update_curr_proc_time();    //aggiorno il cronometro del processo

    state_t *exceptionState = (state_t *)BIOSDATAPAGE;
    int causeCode = CAUSE_GET_EXCCODE(getCAUSE());
    //copy_state(exceptionState, &currentActiveProc->p_s);
    
    if(((exceptionState->status & STATUS_KUp) != ALLOFF) && (causeCode > 0) && (causeCode < 9))
    {
        exceptionState->cause |= (10<<CAUSESHIFT);
        pass_up_or_die(GENERALEXCEPT, exceptionState);
    }

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
            klog_print("\n\nexception_handler error: unknown cause code ");
            klog_print_dec(causeCode);
            PANIC();
    }
}


void interrupt_handler(state_t *excState) {
    int cause = getCAUSE(); // Ritorna il registro CAUSE (3.3 pops)

    if      CAUSE_IP_GET(cause, IL_IPI)         klog_print("interrupt_handler:Il_IPI"); // Ignora intterrupt
    else if CAUSE_IP_GET(cause, IL_CPUTIMER)    plt_time_handler(excState);
    else if CAUSE_IP_GET(cause, IL_TIMER)       intervall_timer_handler(excState);
    else if CAUSE_IP_GET(cause, IL_DISK)        device_handler(IL_DISK, excState);
    else if CAUSE_IP_GET(cause, IL_FLASH)       device_handler(IL_FLASH, excState);
    else if CAUSE_IP_GET(cause, IL_ETHERNET)    device_handler(IL_ETHERNET, excState);
    else if CAUSE_IP_GET(cause, IL_PRINTER)     device_handler(IL_PRINTER, excState);
    else if CAUSE_IP_GET(cause, IL_TERMINAL)    terminal_handler();

    klog_print("\n\ninterrupt_handler error: unknown cause code ");
    klog_print_dec(cause);
    PANIC();
}


void tlb_handler(state_t *excState) {
    pass_up_or_die(PGFAULTEXCEPT, excState);
}


void trap_handler(state_t *excState) {
    pass_up_or_die(GENERALEXCEPT, excState);
}