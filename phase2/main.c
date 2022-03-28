#include "../phase1/headers/asl.h"
#include "../phase1/headers/pcb.h"
#include "../phase1/headers/listx.h"
#include "../generic_headers/pandos_const.h"
#include "../generic_headers/pandos_types.h"
#include "headers/scheduler.h"
#include "p2test.c"
#include "klog.c"

// Dichiarazione della funzione di test contenuta in p2test.c
extern void test();
/* Variabili Globali */
int active_proc;                        //Processi iniziati e non ancora terminati: attivi || Process Count
int blocked_proc;                       //Processi 'blocked': in attesa di I/O oppure timer || Soft-Block Count
struct list_head *ready_queue;          //Coda dei processi ready || Ready-Queue
pcb_t *curr_active_proc;                //Puntatore processo in stato "running" (attivo) || Current Process
short semDev[5];                        //Direi di farlo lungo 5 dato che nella guida c'è scritto: || Device Semaphores
/*
The Nucleus maintains one integer semaphore
for each external (sub)device in µMPS3, plus one additional semaphore
to support the Pseudo-clock.
Since terminal devices are actually two independent sub-devices,
the Nucleus maintains two semaphores for each terminal device.
Dunque sono 2 semafori per ogni terminal device (2) quindi 4 più il semaforo degli pseudo-clock = 5.
Fatemi sapere se vi quadra
*/

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
    vector = (passupvector_t *) PASSUPVECTOR;  
    vector->tlb_refill_handler = (memaddr) uTLB_RefillHandler;
    vector->tlb_refill_stackPtr = KERNELSTACK;
    //TODO la seguente assegnazione è sbagliata. Va assegnato alla funzione di gestione delle eccezzioni(da creare)
    vector->exception_handler = (memaddr) (PASSUPVECTOR + KUPBITON);
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


    //Alloc low priority process
    pcb_PTR firstProc = allocPcb();
    insertProcQ(ready_queue, firstProc);
    firstProc->p_prio = 0; // Setto priorità bassa

    firstProc->p_s.cause = 0;

    // Inizializzazione processo
    firstProc->p_parent = NULL;
    firstProc->p_child.next = NULL; //Settare direttamente p_child a NULL generava errori
    firstProc->p_child.prev = NULL; 
    firstProc->p_sib.next = NULL; //Stesso discorso di sopra
    firstProc->p_sib.prev = NULL;
    
    firstProc->p_time = 0;
    firstProc->p_semAdd = NULL;
    firstProc->p_supportStruct = NULL;

    ++active_proc; 

    //TODO
    /*
    Nella fase di inizializzazione manca da fare ciò che è scritto qua.
    Purtroppo non ho ancora capito come metterci mano.
    Comunque sappiate che firstProc->p_s.Parametri_di_Ps funziona correttamente
    anche se visual studio non ve lo suggerisce (o almeno a me.)

    */






    return 0;
}