#include "headers/scheduler.h"

#define SEMDEVLEN 49

extern int activeProc;
extern int blockedProc;
extern struct list_head queueLowProc;
extern struct list_head queueHighProc;
extern pcb_t *currentActiveProc;
extern short semDevice[SEMDEVLEN];


void scheduler() {
    pcb_PTR p;

    // Scheduling dei processi ad alta priorita'
    if((p = removeProcQ(&queueHighProc)) != NULL) {
        currentActiveProc = p;
        LDST(&p->p_s);
    } else {

        // Scheduling dei processi a bassa priorita'
        if ((p = removeProcQ(&queueLowProc)) != NULL) {
            currentActiveProc = p;
            //Load 5 milliseconds on the PLT.
            LDST(&p->p_s);
        } else {
            // Se le code dei processi ready sono vuote, esegui i seguenti controlli
            
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
}