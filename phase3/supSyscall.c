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
        sems = &semPrinter_phase3[i];
        break;

    case IL_TERMINAL:
        sems = &semTermWrite_phase3[i];
        break;

    default:
        PANIC();
        break;
    }

    return sems;
}

/**
 * It writes a string to a device
 *
 * @param s the support structure
 * @param IL_X the device type (terminal or disk)
 */
void write(support_t *s, int mode)
{
    char *msg = (char *)s->sup_exceptState[GENERALEXCEPT].reg_a1;
    int arg1, arg2,
        len = s->sup_exceptState[GENERALEXCEPT].reg_a2,
        terminal = mode == IL_TERMINAL,
        index = s->sup_asid - 1;
    unsigned int status, cond = (terminal ? TERMSTATMASK : -1);

    int *sem = get_dev_sem(index, mode);
    void *device = (void *)DEV_REG_ADDR(mode, index);

    if (len < 0 || len > MAXSTRLENG || (memaddr)msg < KUSEG)
        trap();

    SYSCALL(PASSEREN, (int)sem, 0, 0);

    for (int i = 0; i < len; i++)
    {
        arg1 = (int)&((dtpreg_t *)device)->command;
        arg2 = PRINTCHR;

        if (terminal)
        {
            arg1 = (int)&((termreg_t *)device)->transm_command;
            arg2 |= ((unsigned int)msg[i] << 8);
        }
        else
            ((dtpreg_t *)device)->data0 = msg[i];

        status = SYSCALL(DOIO, arg1, arg2, 0);

        if ((status & cond) != (terminal ? RECVD : READY))
        {
            len = -(status & cond);
            break;
        }
    }

    SYSCALL(VERHOGEN, (int)sem, 0, 0);

    s->sup_exceptState[GENERALEXCEPT].reg_v0 = len;
}

/**
 * It reads from the terminal, and stores the read characters in the virtual address passed as argument
 *
 * @param sup the support structure
 * @param virtualAddr the address of the first byte of the buffer where the input will be stored
 */
void read_from_terminal(support_t *sup, char *addr)
{
    if ((memaddr)addr < KUSEG) /* indirizzo out memoria virtuale / o lunghezza richiesta 0 */
        trap();

    int len = 0, tmp = 0, index = sup->sup_asid - 1; // legge da 1 a 8 (ASID), ma i devices vanno da 0 a 7
    int *sem = &semTermRead_phase3[index];
    termreg_t *termDev = (termreg_t *)(DEV_REG_ADDR(IL_TERMINAL, index));

    SYSCALL(PASSEREN, (int)sem, 0, 0);

    while (tmp != '\n')
    {
        int status = SYSCALL(DOIO, (int)&(termDev->recv_command), RECEIVECHAR, 0);
        if ((status & 0xFF) != CHARRECV)
        {
            len = -1 * status;
            break;
        }
        *addr = status >> BYTELENGTH;
        tmp = status >> BYTELENGTH;
        addr++;
        len++;
    }

    SYSCALL(VERHOGEN, (int)sem, 0, 0);

    sup->sup_exceptState[GENERALEXCEPT].reg_v0 = len;
}
