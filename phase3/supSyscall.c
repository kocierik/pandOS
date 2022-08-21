#include "headers/supSyscall.h"

#define PRINTCHR 2
#define TERMSTATMASK 0xFF
#define DEV_STATUS_READY 1

int sem_term[UPROCMAX], sem_print[UPROCMAX];

void get_tod(support_t *s)
{
    cpu_t tod;
    STCK(tod);
    s->sup_exceptState[GENERALEXCEPT].reg_v0 = tod;
}

void terminate(support_t *s)
{
    SYSCALL(VERHOGEN, (int)&swap_pool_sem, 0, 0); // release swap pool semaphore
    for (int i = 0; i < POOLSIZE; ++i)
        if (swap_pool_table[i].sw_asid == s->sup_asid)
            swap_pool_table[i].sw_asid = NOPROC; // unmark swap pool table
    free_sd(s);                                  // free support descriptor
    SYSCALL(VERHOGEN, (int)&master_sem, 0, 0);   // realise master sem
    SYSCALL(TERMPROCESS, 0, 0, 0);
}

void write_to_printer(support_t *s)
{
    syscall_write(s, IL_PRINTER);
}

void write_to_terminal(support_t *s)
{
    syscall_write(s, IL_TERMINAL);
}

void syscall_write(support_t *s, int IL_X)
{
    int len, terminal = FALSE;
    int index = s->sup_asid - 1;
    int *sems;

    char *msg = (char *)s->sup_exceptState[GENERALEXCEPT].reg_a1;
    len = s->sup_exceptState[GENERALEXCEPT].reg_a2;

    if (len < 0 || len > 100 || (memaddr)msg < KUSEG)
        SYSCALL(TERMINATE, 0, 0, 0);

    void *device = (void *)DEV_REG_ADDR(IL_X, index);

    switch (IL_X)
    {
    case IL_PRINTER:
        sems = sem_print;
        break;

    case IL_TERMINAL:
        sems = sem_term;
        terminal = TRUE;
        break;

    default:
        PANIC();
        break;
    }

    int sem = sems[index];

    SYSCALL(PASSEREN, (int)&sem, 0, 0);

    for (int i = 0; i < len; i++)
    {
        int arg1 = (int)&((dtpreg_t *)device)->command;
        int arg2 = PRINTCHR;

        if (terminal) {
            arg1 = (int)&((termreg_t *)device)->transm_command;
            arg2 |= ((unsigned int)msg[i] << 8);
        } else {
            ((dtpreg_t *)device)->data0 = msg[i];
            arg2 |= 0;
        }

        unsigned int status = SYSCALL(DOIO, arg1, (int)arg2, 0);

        if ((status & (terminal ? TERMSTATMASK : -1)) != (terminal ? RECVD : DEV_STATUS_READY))
        {
            len = -(status & (terminal ? TERMSTATMASK : -1));
            break;
        }
    }

    SYSCALL(VERHOGEN, (int)&sem, 0, 0);

    s->sup_exceptState[GENERALEXCEPT].reg_v0 = len;
}

void read_from_terminal(support_t *s)
{
    int ret = -1;
    // ??
    s->sup_exceptState[GENERALEXCEPT].reg_v0 = ret;
}