#include "headers/scheduler.h"

#define SEMDEVLEN 49

extern int activeProc;
extern int blockedProc;
extern struct list_head queueLowProc;
extern struct list_head queueHighProc;
extern pcb_t *currentActiveProc;
extern short semDevice[SEMDEVLEN];


void scheduler() {
    
    pcb_PTR p; //Puntatore al processo che sto per prendere in carico. 

    // Scheduling dei processi ad alta priorita'
    /*
    * Verifico che la coda dei processi ad alta priorità non sia vuota. Se non lo è estraggo il processo che è li ad attendere;
    * e lo assegno ad una variabile indicante il processo correntemente attivo. 
    */
    if((p = removeProcQ(&queueHighProc)) != NULL) { 
        currentActiveProc = p;
        LDST(&p->p_s);
    } else if ((p = removeProcQ(&queueLowProc)) != NULL) {
        // Scheduling dei processi a bassa priorita'
        currentActiveProc = p;
        //Load 5 milliseconds on the PLT.
        setTIMER(5);
        LDST(&p->p_s);
    } else {
        /*
        * A questo punto se vi era un processo in una delle code (Alta o Bassa priorità) questo sarà stato passato
        * dallo stato di "ready" allo stato "running".
        * Altrimenti, se le code dei processi "ready" sono vuote, eseguo i seguenti controlli.       
        */
        
        if(activeProc == 0)
            HALT(); //Il processore non deve fare niente quindi si ferma
            
        if(activeProc > 0 && blockedProc > 0) {
            /* TODO
            IMPORTANT POINT
            - set the Status register to enable interrupts
            - disable the PLT (also through the Status register), or load it with a very large value
            
            The first interrupt that occurs after entering a Wait State should not be for the PLT.
            */
            //(memaddr) LOCALTIMERINT = 0xFFFFFFFF;
            WAIT(); //twiddling its thumbs
        }

        if(activeProc > 0 && blockedProc == 0) {
            /* Take an appropriate deadlock detected action;   -> che minchia vuol dire questa cosa
                invoke the PANIC BIOS service/instruction.  */
            PANIC();        //DEADLOCK
        }
        
    }
}