#include "../phase1/headers/asl.h"
#include "../phase1/headers/pcb.h"
#include "../phase1/headers/listx.h"
#include "../generic_headers/pandos_const.h"
#include "../generic_headers/pandos_types.h"
#include "headers/scheduler.h"
#include "p2test.c"
#include "klog.c"

/* Variabili Globali */
int active_proc;                        //Processi iniziati e non ancora terminati: attivi
int blocked_proc;                       //Processi 'blocked': in attesa di I/O oppure timer.
static LIST_HEAD(ready_proc);           //Coda dei processi ready
pcb_t *curr_active_proc;                //Puntatore processo in stato "running" (attivo)
short semDev[MAXSEM];             //NON SO DI QUANTO FARLO LUNGO HELP


void initGlobalVar() {
    active_proc  = 0;
    blocked_proc = 0;
    curr_active_proc = NULL;
    for (int i = 0; i < MAXSEM; ++i) {
        semDev[i] = 0;
    }
}


void initPassUpVector(passupvector_t *vector) {
    vector = (passupvector_t *) PASSUPVECTOR;  
    vector->tlb_refill_handler = (memaddr) uTLB_RefillHandler;
    vector->tlb_refill_stackPtr = KERNELSTACK;
    vector->exception_handler = (memaddr) (PASSUPVECTOR + KUPBITON); //Basato su Table pag 89 pops but non sicuro
    vector->exception_stackPtr = KERNELSTACK;
}


int main(int argc, int* argv[]){

    //Inizializzazione fase 1
    initPcbs();
    initASL();

    //Inizializzare le variabili dichiarate precedentemente
    initGlobalVar();

    
    /* Pass Up Vector */
    passupvector_t *vector;
    initPassUpVector(vector);

    LDIT(100); //imposto l'inteval timer a 100 ms

    /*
    Instantiate a single low priority process, place its
    pcb in the Ready Queue, and increment Process Count.
    A process is instantiated by allocating a pcb
    (i.e. allocPcb()), and initializing the processor state
    that is part of the pcb
    */

    //alloc low priority process
    pcb_PTR firstProc = allocPcb();
    insertProcQ(&ready_proc, firstProc);
    ++active_proc;


    /*
    In particular this process needs to have interrupts enabled,
    the processor Local Timer enabled, kernel-mode on, the
    SP set to RAMTOP (i.e. use the last RAM frame for its stack), and
    its PC set to the address of test

    firstProc->p_s.
    */

    //scheduler();

    return 0;
}