#include "headers/scheduler.h"

// TEMPORARY
extern void klog_print(char *s);


extern int activeProc;
extern int blockedProc;
extern struct list_head queueLowProc;
extern struct list_head queueHighProc;
extern pcb_t *currentActiveProc;


void scheduler() {
    
    pcb_PTR p;
    
    /*
    cpu_t startCpuTime=1;
    cpu_t cpuTime;
    STCK(cpuTime); // Tempo attuale della cpu
    if(currentActiveProc != NULL)
        currentActiveProc->p_time += cpuTime - startCpuTime;
    
    * Verifico che la coda dei processi ad alta priorità non sia vuota.
    * Se non lo è estraggo il processo che è li ad attendere;
    * e lo assegno ad una variabile indicante il processo correntemente attivo. 
    */

    if((p = removeProcQ(&queueHighProc)) != NULL) {
        currentActiveProc = p;
        LDST(&(p->p_s));

    } else if ((p = removeProcQ(&queueLowProc)) != NULL) {
        currentActiveProc = p;
        //Load 5 milliseconds on the PLT
        setTIMER(TIMESLICE);  //TODO: DA CONTROLLARE
        //STCK(startCpuTime);
        LDST(&p->p_s);

    } else {
        /*
        * A questo punto se c'era un processo in una delle due code questo sarà stato passato
        * dallo stato di "ready" allo stato "running".
        * Altrimenti, se le code dei processi "ready" sono vuote, eseguo i seguenti controlli.       
        */

        if(activeProc == 0)
            HALT(); //Il processore non deve fare niente quindi si ferma
            
        if(activeProc > 0 && blockedProc > 0) {
            //Enabling interrupts and disable PLT.
            STATE_PTR status = (STATE_PTR)(ALLOFF | IEPON | IMON);
            STST(status);  // TODO: DA CONTROLLARE, si può usare setStatus???
            WAIT(); //twiddling its thumbs
        }

        if(activeProc > 0 && blockedProc == 0)
            PANIC();        //DEADLOCK
    }
}