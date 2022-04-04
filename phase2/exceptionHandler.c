#include "headers/exceptionHandler.h"
#include "klog.c"


void exceptionHandler() {
    //klog_print("Dentro Exception Handler\n\n"); //TODO rimuovi
    //Stato al momento dell'eccezione; utile dopo per capire se la chiamata alla SysCall è avvenuta in Kernel mode o no.
    state_t *exceptionState = (state_t *) BIOSDATAPAGE;
    
    switch(CAUSE_GET_EXCCODE(getCAUSE())){
        case IOINTERRUPTS: // Interrupt
            interruptHandler();
            break;
        case 1:         // TLB Exception
        case TLBINVLDL:
        case TLBINVLDS: 
            TLBHandler();
            break;
        case SYSEXCEPTION: // Chiamata una System Call
            // Bisogna capire qui se la SysCall è chiamata in Kernel Mode o no. Non so come si fa ad ora.
            syscall_handler(exceptionState);
            break;
        default: // TRAP
            trapHandler();
            break;
    }
}