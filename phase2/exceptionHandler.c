#include "headers/exceptionHandler.h"
#include "umps3/umps/cp0.h"


void exceptionHandler() {
    //Stato al momento dell'eccezione; utile dopo per capire se la chiamata alla SysCall è avvenuta in Kernel mode o no.
    state_t *exceptionState = (state_t *) BIOSDATAPAGE; 

    // TODO: capire come e dove prendere questo codice
    int syscode = -1; //codice della syscall da prendere dal campo a0 dello stato del current process
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
            // TODO
            // Bisogna capire qui se la SysCall è chiamata in Kernel Mode o no. Non so come si fa ad ora.
            syscall_handler(syscode);
            break;
        default: // TRAP
            trapHandler();
            break;
    }
}