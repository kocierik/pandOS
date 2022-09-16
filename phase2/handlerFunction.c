#include "headers/handlerFunction.h"

/* Usefull external function */
extern void insert_ready_queue(int prio, pcb_PTR p);

// vector for Bitwise operation
int powOf2[] = {1, 2, 4, 8, 16, 32, 64, 128, 256};

/* INTERRUPT HANDLER FUNCTION */

// handler IL_CPUTIMER
void plt_time_handler(state_t *excState)
{
    klog_print("plt exc\n");
    setTIMER(-2); // ACK
    copy_state(excState, &currentActiveProc->p_s);
    insert_ready_queue(currentActiveProc->p_prio, currentActiveProc);
    scheduler();
}

// handler IL_TIMER
void intervall_timer_handler(state_t *excState)
{
    klog_print("timer exc\n");
    LDIT(100000); // ACK
    pcb_PTR p;
    while ((p = removeBlocked(&semIntervalTimer)) != NULL)
    {
        --blockedProc;
        insert_ready_queue(p->p_prio, p);
    }
    semIntervalTimer = 0;
    load_or_scheduler(excState);
}

/**
 * Obtain the semaphore address of a generic device (NON TERMINAL)
 * Take the device's interrupt line and the device number as input (from 0 to 7)
 * and return semaphore's address
 *
 * @param interruptLine The interrupt line yo recognize.
 * @param devNumber The device index of the semaphore array
 *
 * @return A pointer to the semaphore for the device.
 */
int *getDeviceSemaphore(int interruptLine, int devNumber)
{
    switch (interruptLine)
    {
    case IL_DISK:
        return &semDiskDevice[devNumber];
    case IL_FLASH:
        return &semFlashDevice[devNumber];
    case IL_ETHERNET:
        return &semNetworkDevice[devNumber];
    case IL_PRINTER:
        return &semPrinterDevice[devNumber];
    }
    return NULL;
}

/**
 * It returns the device number of the interrupt line that is currently active
 *
 * @param interLine the interrupt line that the device is on
 *
 * @return The device number.
 */
/**
int getDevice(int interLine)
{
    klog_print("\n\n");
    for (int i = 0; i < 8; i++)
    {
        if (interLine & powOf2[i]){
            g2 = powOf2[i];
            klog_print("GetDevice sta per tornare: ");
            klog_print_dec(i);
            return i;
        }
    }
    klog_print("Errore get Device ");
    return -1; // ERROR
}
*/
// handler IL_DISK | IL_FLASH | IL_ETHERNET | IL_PRINTER | IL_TERMINAL
void device_handler(int interLine, state_t *excState)
{
    unsigned int statusCode;
    int *devSemaphore = NULL; // Semaphore address
    int devNumber = 0;


    devregarea_t *dev_regs = (devregarea_t *)RAMBASEADDR;
    unsigned int bitmap_word = dev_regs->interrupt_dev[interLine - 3];
    unsigned int mask = 1;

    // Scorro i dispositivi
    for (int i = 0; i < N_DEV_PER_IL; i++)
    {
        // Check per trovare il device con un pending interrupt
        if (bitmap_word & mask)
        {
            // Salvo il numero del device trovato
            devNumber = i;

            if (interLine == IL_TERMINAL)
            {
                termreg_t *devRegAddr = (termreg_t *)DEV_REG_ADDR(interLine, devNumber);

                if (devRegAddr->recv_status == RECVD)
                {
                    statusCode = devRegAddr->recv_status;
                    devRegAddr->recv_command = ACK;
                    devSemaphore = &semTerminalDeviceReading[devNumber];
                }
                else
                {
                    statusCode = devRegAddr->transm_status;
                    devRegAddr->transm_command = ACK;
                    devSemaphore = &semTerminalDeviceWriting[devNumber];
                }
            }
            else
            {
                dtpreg_t *devRegAddr = (dtpreg_t *)DEV_REG_ADDR(interLine, devNumber); // da controllare
                devSemaphore = getDeviceSemaphore(interLine, devNumber);               // errore qua
                statusCode = devRegAddr->status;                                       // Save status code
                devRegAddr->command = ACK;                                             // Acknowledge the interrupt
            }
        }
        mask *= 2;
    }

    /* V-Operation */
    pcb_PTR p = V(devSemaphore, NULL);

    if (p == currentActiveProc)
    {
        if (p == NULL)
            klog_print("Errore TRAP fatale\n");
        // trap(); // ERRORE
        currentActiveProc->p_s.reg_v0 = statusCode; // si blocca qua
        insert_ready_queue(currentActiveProc->p_prio, currentActiveProc);
        scheduler();
    }
    else
    {
        p->p_s.reg_v0 = statusCode;
        load_or_scheduler(excState);
    }
}

// Handler TLB | TRAP
void pass_up_or_die(int pageFault, state_t *excState)
{
    if (currentActiveProc != NULL)
    {
        if (currentActiveProc->p_supportStruct == NULL)
        {
            klog_print("die \n");
            term_proc(0);
            scheduler();
        }
        else
        {
            klog_print("passup \n");
            copy_state(excState, &currentActiveProc->p_supportStruct->sup_exceptState[pageFault]);
            int stackPtr = currentActiveProc->p_supportStruct->sup_exceptContext[pageFault].stackPtr;
            int status = currentActiveProc->p_supportStruct->sup_exceptContext[pageFault].status;
            int pc = currentActiveProc->p_supportStruct->sup_exceptContext[pageFault].pc;
            LDCXT(stackPtr, status, pc);
        }
    }
}