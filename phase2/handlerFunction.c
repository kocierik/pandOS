#include "headers/handlerFunction.h"

/* Usefull external variables */
extern int blockedProc;
extern pcb_t *currentActiveProc;
extern int semIntervalTimer;
extern int semDiskDevice[8];
extern int semFlashDevice[8];
extern int semNetworkDevice[8];
extern int semPrinterDevice[8];
extern int semTerminalDeviceReading[8];
extern int semTerminalDeviceWriting[8];

/* Usefull external function */
extern void insert_ready_queue(int prio, pcb_PTR p);

// vector for Bitwise operation
int powOf2[] = {1, 2, 4, 8, 16, 32, 64, 128, 256};

/* INTERRUPT HANDLER FUNCTION */

// handler IL_CPUTIMER
void plt_time_handler(state_t *excState)
{
    setTIMER(-2); // ACK
    copy_state(excState, &currentActiveProc->p_s);
    insert_ready_queue(currentActiveProc->p_prio, currentActiveProc);
    scheduler();
}

// handler IL_TIMER
void intervall_timer_handler(state_t *excState)
{
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
int getDevice(int interLine)
{
    for (int i = 0; i < 8; i++)
    {
        if (interLine & powOf2[i])
            return i;
    }
    return -1; // ERROR
}

// handler IL_DISK | IL_FLASH | IL_ETHERNET | IL_PRINTER
void device_handler(int interLine, state_t *excState)
{
    unsigned int statusCode;
    int *deviceSemaphore;
    int devNumber = getDevice(interLine);

    if (interLine == IL_TERMINAL)
    {
        termreg_t *devRegAddr = (termreg_t *)(0x10000054 + ((IL_TERMINAL - 3) * 0x80) + (devNumber * 0x10));

        if (devRegAddr->recv_status == RECVD)
        {
            statusCode = devRegAddr->recv_status;
            devRegAddr->recv_command = ACK;
            deviceSemaphore = &semTerminalDeviceReading[devNumber];
        }
        else
        {
            statusCode = devRegAddr->transm_status;
            devRegAddr->transm_command = ACK;
            deviceSemaphore = &semTerminalDeviceWriting[devNumber];
        }
    }
    else
    {
        dtpreg_t *devRegAddr = (dtpreg_t *)((0x10000054 + ((interLine - 3) * 0x80) + (devNumber * 0x10)));
        deviceSemaphore = getDeviceSemaphore(interLine, devNumber);
        statusCode = devRegAddr->status; // Save status code
        devRegAddr->command = ACK;       // Acknowledge the interrupt
    }

    /* V-Operation */
    pcb_PTR p = V(deviceSemaphore, NULL);

    if (p == NULL || p == currentActiveProc)
    {
        currentActiveProc->p_s.reg_v0 = statusCode;
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
            term_proc(0);
            scheduler();
        }
        else
        {
            copy_state(excState, &currentActiveProc->p_supportStruct->sup_exceptState[pageFault]);
            int stackPtr = currentActiveProc->p_supportStruct->sup_exceptContext[pageFault].stackPtr;
            int status = currentActiveProc->p_supportStruct->sup_exceptContext[pageFault].status;
            int pc = currentActiveProc->p_supportStruct->sup_exceptContext[pageFault].pc;
            LDCXT(stackPtr, status, pc);
        }
    }
}