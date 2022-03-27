#include "headers/scheduler.h"


void scheduler() {

    /*
    If the queue of high priority processes is not empty:
        1. Remove the pcb from the head of the high priority Ready Queue
           and store the pointer to the pcb in the Current Process field.
        2. Perform a Load Processor State (LDST) on the processor state
           stored in pcb of the Current Process (p s).
    otherwise
        1. Remove the pcb from the head of the low priority Ready Queue
           and store the pointer to the pcb in the Current Process field.
        2. Load 5 milliseconds on the PLT. [Section 4.1.4-pops]
        3. Perform a Load Processor State (LDST) on the processor state
           stored in pcb of the Current Process (p s).
    */
    LDST(1);  // funzione di libumps
}