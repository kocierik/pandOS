#include "../phase1/headers/asl.h"
#include "../phase1/headers/pcb.h"
#include "../phase1/headers/listx.h"
#include "../generic_headers/pandos_const.h"
#include "../generic_headers/pandos_types.h"

// Dichiarazione delle funzioni esterne
extern void test();
extern void exceptionHandling();
extern void scheduler();
extern void uTLB_RefillHandler();

/* Variabili Globali */
int active_proc;                    //Processi iniziati e non ancora terminati: attivi || Process Count
int blocked_proc;                   //Processi 'blocked': in attesa di I/O oppure timer || Soft-Block Count
struct list_head *ready_queue;      //Coda dei processi ready || Ready-Queue
pcb_t *curr_active_proc;            //Puntatore processo in stato "running" (attivo) || Current Process
short semDev[5];                    //Direi di farlo lungo 5 dato che nella guida c'è scritto: || Device Semaphores


void initGlobalVar() {
    active_proc  = 0;
    blocked_proc = 0;
    mkEmptyProcQ(ready_queue); 
    curr_active_proc = NULL;
    for (int i = 0; i < 5; ++i) {
        semDev[i] = 0;
    }
}


void initPassUpVector(passupvector_t *vector) {
    vector->tlb_refill_handler = (memaddr) uTLB_RefillHandler;
    vector->tlb_refill_stackPtr = KERNELSTACK;
    vector->exception_handler = (memaddr) (exceptionHandling);
    vector->exception_stackPtr = KERNELSTACK;
}


int main(int argc, int* argv[]){

    //Inizializzare le variabili dichiarate precedentemente
    initGlobalVar();

    //Inizializzazione fase 1
    initPcbs();
    initASL();
    
    /* Pass Up Vector */
    passupvector_t *vector = (passupvector_t *) PASSUPVECTOR;
    initPassUpVector(vector);

    LDIT(100000); //imposto l'interval timer a 100 ms


    //Alloc low priority process
    pcb_PTR firstProc = allocPcb();

    insertProcQ(ready_queue, firstProc);
    ++active_proc; 

    // Inizializzazione processo
    firstProc->p_prio = PROCESS_PRIO_LOW; // Setto priorità bassa

    //Status Interrupt Mask, Local Timer, Kernel Mode
    firstProc->p_s.status = ALLOFF | IEPON | TEBITON | IMON;
    RAMTOP(firstProc->p_s.reg_sp);
    firstProc->p_s.reg_t9 = (memaddr) test;
    firstProc->p_s.pc_epc = (memaddr) test;
    
    STST(&firstProc->p_s);  
    
    scheduler();

    return 0;
}