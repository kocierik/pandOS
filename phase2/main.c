#include <umps/libumps.h>
#include "../generic_headers/pandos_const.h"
#include "../generic_headers/pandos_types.h"
#include "../phase1/headers/asl.h"
#include "../phase1/headers/pcb.h"
#include "../phase1/headers/listx.h"
#include "headers/exceptionHandler.h"
#include "headers/scheduler.h"
#include "p2test.c"


/* Variabili Globali */
int processId;
int activeProc;                 //Processi iniziati e non ancora terminati: attivi || Process Count
int blockedProc;                //Processi 'blocked': in attesa di I/O oppure timer || Soft-Block Count
LIST_HEAD(queueLowProc);        //Coda dei processi a bassa priorità
LIST_HEAD(queueHighProc);       //Coda dei processi a alta priorità
pcb_t *currentActiveProc;       //Puntatore processo in stato "running" (attivo) || Current Process
short semDevice[SEMDEVLEN];     //Semplice intero per i semafori dei device|| Device Semaphores


void initGlobalVar() {
    processId = -1;
    activeProc  = 0;
    blockedProc = 0;
    currentActiveProc = NULL;
    for (int i = 0; i < SEMDEVLEN; ++i)
        semDevice[i] = 0;
}


void initPassUpVector(passupvector_t *vector) {
    vector->tlb_refill_handler = (memaddr) uTLB_RefillHandler;
    vector->tlb_refill_stackPtr = KERNELSTACK;
    vector->exception_handler = (memaddr) (exceptionHandler); //TODO: FUNZIONE DA FARE/CONTROLLIAMO CHE SIA CORRETTO
    vector->exception_stackPtr = KERNELSTACK;
}


void insertReadyQueue(int prio, pcb_PTR p) {
    ++activeProc;
    if(prio == PROCESS_PRIO_LOW)
        insertProcQ(&queueLowProc, p);
    else
        insertProcQ(&queueHighProc, p);
}

void assegnaPID(pcb_PTR p) {
    p->p_pid = ++processId;
}


int main(int argc, int* argv[]){

    //Inizializzazione fase 1
    initPcbs();
    initASL();

    //Inizializzare le variabili dichiarate precedentemente
    initGlobalVar();
    
    /* Pass Up Vector */
    passupvector_t *vector = (passupvector_t *) PASSUPVECTOR;
    initPassUpVector(vector);

    LDIT(100000); //imposto l'interval timer a 100 ms

    //Alloc low priority process
    pcb_PTR firstProc = allocPcb();
    assegnaPID(firstProc);


    firstProc->p_prio = PROCESS_PRIO_LOW; //Setto priorità bassa
    
    //Inserisco il processore nella coda dei processi Ready
    insertReadyQueue(firstProc->p_prio, firstProc);

    insertProcQ(&queueLowProc, firstProc);
    ++activeProc; 


    //Setto lo status del processo con i bit di Interrupt Mask, Local Timer e Kernel Mode
    firstProc->p_s.status = ALLOFF | IEPON | IMON | TEBITON;

    //Setto il PC (e t9) del proccesso a test
    firstProc->p_s.pc_epc = (memaddr) test;
    firstProc->p_s.reg_t9 = (memaddr) test;

    //Carico SP in cima alla RAM e lo status del processo nel processore
    RAMTOP(firstProc->p_s.reg_sp);
    STST(&firstProc->p_s);

    scheduler();

    return 0;
}