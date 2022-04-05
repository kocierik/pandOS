#include <umps/libumps.h>
#include "../generic_headers/pandos_const.h"
#include "../generic_headers/pandos_types.h"
#include "../phase1/headers/asl.h"
#include "../phase1/headers/pcb.h"
#include "../phase1/headers/listx.h"

/* Extern functions */
extern void test();
extern void uTLB_RefillHandler();
extern void exceptionHandler();
extern void scheduler();

// TEMPORARY
extern void klog_print(char *s);

/* Global Variables */
static int processId;           // Variabile globale utilizzata per assegnare un id unico ai processi creati
int activeProc;                 // Processi iniziati e non ancora terminati: attivi || Process Count
int blockedProc;                // Processi 'blocked': in attesa di I/O oppure timer || Soft-Block Count
struct list_head queueLowProc;  // Coda dei processi a bassa priorità
struct list_head queueHighProc; // Coda dei processi a alta priorità
pcb_PTR currentActiveProc;      // Puntatore processo in stato "running" (attivo) || Current Process
int semDevice[SEMDEVLEN];       // Vettore di interi per i semafori dei device|| Device Semaphores


void initGlobalVar() {
    processId = 0;
    activeProc  = 0;
    blockedProc = 0;
    mkEmptyProcQ(&queueLowProc);
    mkEmptyProcQ(&queueHighProc);
    currentActiveProc = NULL;
    for (int i = 0; i < SEMDEVLEN; ++i)
        semDevice[i] = 0;
}


void initPassUpVector(passupvector_t *vector) {
    vector->tlb_refill_handler = (memaddr) uTLB_RefillHandler;
    vector->tlb_refill_stackPtr = KERNELSTACK;
    vector->exception_handler = (memaddr) (exceptionHandler);
    vector->exception_stackPtr = KERNELSTACK;
}


// inserisco un processo nella giusta coda e assegno la priorità al processo
void insertReadyQueue(int prio, pcb_PTR p) {
    p->p_prio = prio;
    ++activeProc;
    if(prio == PROCESS_PRIO_HIGH)
        insertProcQ(&queueHighProc, p);
    else
        insertProcQ(&queueLowProc, p);
}


//funzione di aiuto che assegna un id unico a un processo 
void assegnaPID(pcb_PTR p) {
    p->p_pid = ++processId;
}


int main(int argc, int* argv[]){

    /* Inizializzazione variabili */
    initPcbs();
    initASL();
    initGlobalVar();

    klog_print("Variabili inizializzate...\n\n");
    
    /* Pass Up Vector */
    passupvector_t *vector = (passupvector_t *) PASSUPVECTOR;
    initPassUpVector(vector);

    klog_print("Pass Up Vector inizializzato...\n\n");

    LDIT(100000); //imposto l'interval timer a 100 ms

    /* Allocchiamo il primo processo a bassa priorita' e settiamo le cose giuste */
    pcb_PTR firstProc = allocPcb();
    assegnaPID(firstProc);

    insertReadyQueue(PROCESS_PRIO_LOW, firstProc);
    firstProc->p_s.status = IEPON | IMON | TEBITON;
    firstProc->p_s.pc_epc = firstProc->p_s.reg_t9 = (memaddr) test;
    RAMTOP(firstProc->p_s.reg_sp);

    klog_print("Primo processo creato, chiamo lo scheduler...\n\n");

    scheduler();

    klog_print("ERRORE: NON DEVO MAI ARRIVARE QUA\n"); 
    return 0;
}