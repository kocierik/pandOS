#include <umps/libumps.h>
#include "headers/mainUsefulFun.h"

int main(int argc, int* argv[]){

    /* Variable initialization */
    initPcbs();
    initASL();
    init_global_var();

    /* Pass Up Vector */
    passupvector_t *vector = (passupvector_t *)PASSUPVECTOR;
    init_passupvector(vector);

    /* Set interval timer to 100ms */
    LDIT(100000); 

    /* Insert first low priority process */
    pcb_PTR firstProc = allocPcb();

    ++activeProc;
    insert_ready_queue(PROCESS_PRIO_LOW, firstProc);
    firstProc->p_s.status = ALLOFF | IEPON | IMON | TEBITON;
    firstProc->p_s.pc_epc = firstProc->p_s.reg_t9 = (memaddr) test;
    RAMTOP(firstProc->p_s.reg_sp);

    scheduler();

    return 0;
}