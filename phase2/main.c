#include "../phase1/headers/asl.h"
#include "../phase1/headers/pcb.h"
#include "../phase1/headers/listx.h"
#include "../generic_headers/pandos_const.h"
#include "../generic_headers/pandos_types.h"
#include "p2test.c"
#include "klog.c"


int active_proc;                //Processi iniziati e non ancora terminati: attivi
int blocked_proc;               //Processi 'blocked': in attesa di I/O oppure timer.
static LIST_HEAD(ready_proc);   //Coda dei processi ready
pcb_t *curr_active_proc;        //Puntatore processo in stato "running" (attivo)


struct semDev {
    unsigned short value;       // boolean value | [0] off | [1] on |
    semd_t sem;
    struct list_head head;
};


void initGlobalVar() {
    active_proc  = 0;
    blocked_proc = 0;
    curr_active_proc = NULL;
}


int main(int argc, int* argv[]){

    //Inizializzazione fase 1
    initPcbs();
    initASL();

    //Inizializzare le variabili dichiarate precedentemente
    initGlobalVar();

    
    /* Pass Up Vector */
    passupvector_t *vector = NULL;  
    vector->tlb_refill_handler = (memaddr) uTLB_RefillHandler;
    vector->tlb_refill_stackPtr = KERNELSTACK;
    vector->exception_handler = (memaddr) (PASSUPVECTOR + KUPBITON); //Basato su Table pag 89 pops but non sicuro
    return 0;
}