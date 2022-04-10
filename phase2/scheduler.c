#include "headers/scheduler.h"
// TEMPORARY
extern void klog_print(char *s);
extern void klog_print_dec(unsigned int num);

extern int activeProc;
extern int blockedProc;
extern struct list_head queueLowProc;
extern struct list_head queueHighProc;
extern pcb_t *currentActiveProc;
extern cpu_t startTime;


void scheduler() {

    pcb_PTR p;
    /*
    * Verifico che la coda dei processi ad alta priorità non sia vuota.
    * Se non lo è estraggo il processo che è li ad attendere;
    * e lo assegno ad una variabile indicante il processo correntemente attivo. 
    */

    if((p = removeProcQ(&queueHighProc)) != NULL) {
        klog_print("\n\nscheduler: ho caricato proc ad alta priorita' con ID -> ");
        klog_print_dec(p->p_pid);
        currentActiveProc = p;
        //setSTATUS(ALLOFF); //suggerito da gian
        STCK(startTime);    // Inizio il cronometro del tempo in cpu
        LDST(&p->p_s);

    } else if ((p = removeProcQ(&queueLowProc)) != NULL) {
        currentActiveProc = p;
        setTIMER(TIMESLICE);    //Load 5 milliseconds on the PLT
        STCK(startTime);    // Inizio il cronometro del tempo in cpu
        LDST(&p->p_s);

    } else {
        /*
        * A questo punto se c'era un processo in una delle due code questo sarà stato passato
        * dallo stato di "ready" allo stato "running".
        * Altrimenti, se le code dei processi "ready" sono vuote, eseguo i seguenti controlli.       
        */
        klog_print("\n\nCode dei processi in attesa vuote...");
        if(activeProc == 0)
            HALT(); //Il processore non deve fare niente quindi si ferma
            
        if(activeProc > 0 && blockedProc > 0) {
            //Enabling interrupts and disable PLT.
            unsigned int status = IEPON | IMON;
            setSTATUS(status);
            WAIT(); //twiddling its thumbs
        }

        if(activeProc > 0 && blockedProc == 0)
            PANIC();        //DEADLOCK
    }
}