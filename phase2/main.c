#include <umps/libumps.h>
#include "../generic_headers/pandos_const.h"
#include "../generic_headers/pandos_types.h"
#include "../phase1/headers/asl.h"
#include "../phase1/headers/pcb.h"
#include "../phase1/headers/listx.h"

/* Extern functions */
extern void test();
extern void test1();
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
cpu_t startTime;

// Vettore di interi per i semafori dei device|| Device Semaphores
/* 
    * Consideriamo ogni coppia di semafori dei terminali come segue: 
    * Primo semaforo dedicato alle operazioni di scrittura (transm)
    * Secondo semaforo dedicato alle operazioni di lettura (recv)
*/  
int semIntervalTimer;
int semDiskDevice[8];
int semFlashDevice[8];
int semNetworkDevice[8];
int semPrinterDevice[8];
int semTerminalDeviceReading[8]; 
int semTerminalDeviceWriting[8];


void initGlobalVar() {
    processId = 0;
    activeProc  = 0;
    blockedProc = 0;
    mkEmptyProcQ(&queueLowProc);
    mkEmptyProcQ(&queueHighProc);
    currentActiveProc = NULL;
    semIntervalTimer = 0;
    for (int i = 0; i < 8; i++) {
        semDiskDevice[i] = 0;
        semFlashDevice[i] = 0;
        semNetworkDevice[i] = 0;
        semPrinterDevice[i] = 0;
        semTerminalDeviceReading[i] = 0;
        semTerminalDeviceWriting[i] = 0;
    }
}


void initPassUpVector(passupvector_t *vector) {
    vector->tlb_refill_handler = (memaddr) uTLB_RefillHandler;
    vector->tlb_refill_stackPtr = KERNELSTACK;
    vector->exception_handler = (memaddr) exceptionHandler;
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

    /* Pass Up Vector */
    passupvector_t *vector = (passupvector_t *)PASSUPVECTOR;
    initPassUpVector(vector);

    LDIT(100000); //imposto l'interval timer a 100 ms

    /* Allocchiamo il primo processo a bassa priorita' e settiamo le cose giuste */
    pcb_PTR firstProc = allocPcb();

    insertReadyQueue(PROCESS_PRIO_LOW, firstProc);
    firstProc->p_s.status = IEPON | IMON | TEBITON;
    firstProc->p_s.pc_epc = firstProc->p_s.reg_t9 = (memaddr) test;
    RAMTOP(firstProc->p_s.reg_sp);

    scheduler();

    return 0;
}