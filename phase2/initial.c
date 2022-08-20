#include <umps/libumps.h>
#include "../generic_headers/pandos_const.h"
#include "../generic_headers/pandos_types.h"
#include "../phase1/headers/asl.h"
#include "../phase1/headers/pcb.h"
#include "../phase1/headers/listx.h"
#include "headers/globals.h"

/* Extern functions */
extern void test();
extern void uTLB_RefillHandler();
extern void exception_handler();
extern void scheduler();

/**
 * It initializes all the global variables
 */
void init_global_var()
{
    processId = 0;
    activeProc = 0;
    blockedProc = 0;
    mkEmptyProcQ(&queueLowProc);
    mkEmptyProcQ(&queueHighProc);
    currentActiveProc = NULL;
    yieldHighProc = NULL;
    semIntervalTimer = 0;
    for (int i = 0; i < 8; i++)
    {
        semDiskDevice[i] = 0;
        semFlashDevice[i] = 0;
        semNetworkDevice[i] = 0;
        semPrinterDevice[i] = 0;
        semTerminalDeviceReading[i] = 0;
        semTerminalDeviceWriting[i] = 0;
    }
}

/**
 * It initializes the passupvector_t structure with the addresses of the exception handler and the TLB
 * refill handler
 *
 * @param vector the address of the passupvector_t structure to initialize
 */
void init_passupvector(passupvector_t *vector)
{
    vector->tlb_refill_handler = (memaddr)uTLB_RefillHandler;
    vector->tlb_refill_stackPtr = KERNELSTACK;
    vector->exception_handler = (memaddr)exception_handler;
    vector->exception_stackPtr = KERNELSTACK;
}

/**
 * It inserts a process into the ready queue, based on its priority
 *
 * @param prio the priority of the process to be inserted
 * @param p the process to be inserted
 */
void insert_ready_queue(int prio, pcb_PTR p)
{
    p->p_prio = prio;
    if (prio == PROCESS_PRIO_HIGH)
        insertProcQ(&queueHighProc, p);
    else
        insertProcQ(&queueLowProc, p);
}

/**
 * It sets the process id of the process pointed to by p to the next available process id
 *
 * @param p the process to set the pid for
 */
void set_pid(pcb_PTR p)
{
    p->p_pid = ++processId;
}

int main(int argc, int *argv[])
{
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
    firstProc->p_s.pc_epc = firstProc->p_s.reg_t9 = (memaddr)test;
    RAMTOP(firstProc->p_s.reg_sp);

    scheduler();

    return 0;
}