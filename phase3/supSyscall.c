#include "headers/supSyscall.h"

/**
 * It gets the time of day from the CPU and puts it in the v0 register
 *
 * @param s the support structure
 */
void get_tod(support_t *s)
{
    cpu_t tod;
    STCK(tod);
    s->sup_exceptState[GENERALEXCEPT].reg_v0 = tod;
}

/**
 * It releases the swap pool semaphore, unmarks the swap pool table, releases the master semaphore,
 * frees the support descriptor and terminates the process
 *
 * @param s the support descriptor
 */
void terminate(support_t *s)
{
    for (int i = 0; i < POOLSIZE; ++i)
        if (swap_pool_table[i].sw_asid == s->sup_asid)
            swap_pool_table[i].sw_asid = NOPROC; // unmark swap pool table
    SYSCALL(VERHOGEN, (int)&master_sem, 0, 0);   // realise master sem
    free_sd(s);                                  // free support descriptor
    klog_print("terminate ");
    SYSCALL(TERMPROCESS, 0, 0, 0);
}

// a write syscall wrapper for printer
void write_to_printer(support_t *s)
{
    write(s, IL_PRINTER);
}

// a write syscall wrapper for terminal
void write_to_terminal(support_t *s)
{
    write(s, IL_TERMINAL);
}

/**
 * It returns a pointer to the semaphore of the device with index i
 *
 * @param i the index of the device
 * @param IL_X the device type, either IL_PRINTER or IL_TERMINAL
 *
 * @return The address of the semaphore for the device.
 */
int *get_dev_sem(int i, int IL_X)
{
    int *sems;
    switch (IL_X)
    {
    case IL_PRINTER:
        sems = semPrinter_phase3;
        break;

    case IL_TERMINAL:
        sems = semTermWrite_phase3;
        break;

    default:
        PANIC();
        break;
    }

    return &sems[i];
}

/**
 * It writes a string to a device
 *
 * @param s the support structure
 * @param IL_X the device type (terminal or disk)
 */
void write(support_t *s, int mode)
{
    unsigned int status;
    char *msg = (char *)s->sup_exceptState[GENERALEXCEPT].reg_a1;
    int arg1, arg2,
        len = s->sup_exceptState[GENERALEXCEPT].reg_a2,
        terminal = mode == IL_TERMINAL, // bool: if is terminal TRUE
        index = s->sup_asid - 1;

    int *sem = get_dev_sem(index, mode);
    void *device = (void *)DEV_REG_ADDR(mode, index);

    if (len < 0 || len > 100 || (memaddr)msg < KUSEG)
        trap();

    SYSCALL(PASSEREN, (int)&sem, 0, 0);

    for (int i = 0; i < len; i++)
    {
        off_interrupts();

        arg1 = (int)&((dtpreg_t *)device)->command;
        arg2 = PRINTCHR;

        if (terminal)
        {
            arg1 = (int)&((termreg_t *)device)->transm_command;
            arg2 |= ((unsigned int)msg[i] << 8);
        }
        else
        {
            ((dtpreg_t *)device)->data0 = msg[i];
            arg2 |= 0; // serve?
        }

        on_interrupts();

        status = SYSCALL(DOIO, arg1, (int)arg2, 0);

        if ((status & (terminal ? TERMSTATMASK : -1)) != (terminal ? RECVD : READY))
        {
            len = -(status & (terminal ? TERMSTATMASK : -1));
            break;
        }
    }

    SYSCALL(VERHOGEN, (int)&sem, 0, 0);

    s->sup_exceptState[GENERALEXCEPT].reg_v0 = len;
}

/**
 * It reads from the terminal, and stores the read characters in the virtual address passed as argument
 *
 * @param sup the support structure
 * @param virtualAddr the address of the first byte of the buffer where the input will be stored
 */
void read_from_terminal(support_t *sup, char *virtualAddr)
{
    if ((memaddr)virtualAddr < KUSEG) /* indirizzo out memoria virtuale / o lunghezza richiesta 0 */
        trap();

    int ret = 0, recv = 0, termASID = sup->sup_asid - 1; // legge da 1 a 8 (ASID), ma i devices vanno da 0 a 7
    int *sem = &semTermRead_phase3[termASID];
    termreg_t *termDev = (termreg_t *)(DEV_REG_ADDR(IL_TERMINAL, termASID));

    SYSCALL(PASSEREN, (int)sem, 0, 0);

    while (recv != '\n')
    {
        int status = SYSCALL(DOIO, (unsigned int)&(termDev->recv_command), RECEIVECHAR, 0);
        if ((status & 0xFF) != CHARRECV)
        {
            ret = -1 * status;
            break;
        }
        *virtualAddr = status >> BYTELENGTH;
        recv = status >> BYTELENGTH;
        virtualAddr++;
        ret++;
    }

    SYSCALL(VERHOGEN, (int)sem, 0, 0);

    sup->sup_exceptState[GENERALEXCEPT].reg_v0 = ret;
}
