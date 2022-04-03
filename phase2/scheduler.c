#include "headers/scheduler.h"

extern void klog_print(char *s);


extern int activeProc;
extern int blockedProc;
extern struct list_head queueLowProc;
extern struct list_head queueHighProc;
extern pcb_t *currentActiveProc;
extern short semDevice[SEMDEVLEN];


void scheduler() {
    klog_print("Entro nello Scheduler\n\n");
    pcb_PTR p; //Puntatore al processo che sto per prendere in carico
    
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

        klog_print("Carico un processo ad alta priorita'\n\n");
        currentActiveProc = p;
        LDST(&(p->p_s));

    } else if ((p = removeProcQ(&queueLowProc)) != NULL) {

        klog_print("Carico un processo a bassa priorita'\n\n");
        currentActiveProc = p;
        //Load 5 milliseconds on the PLT.
        setTIMER(TIMESLICE);  //TODO: DA CONTROLLARE
        klog_print("Timer settato\n\n");
        //STCK(startCpuTime);
        LDST(&p->p_s);

    } else {
        /*
        * A questo punto se c'era un processo in una delle due code questo sarà stato passato
        * dallo stato di "ready" allo stato "running".
        * Altrimenti, se le code dei processi "ready" sono vuote, eseguo i seguenti controlli.       
        */
        klog_print("Non ci sono processi ready\n\n");
        
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