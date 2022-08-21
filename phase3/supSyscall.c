#include "headers/supSyscall.h"


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
    int ret = -1;
    switch (IL_X)
    {
    case IL_PRINTER:
        break;

    case IL_TERMINAL:
        break;

    default:
        PANIC();
        break;
    }
    // ??
    s->sup_exceptState[GENERALEXCEPT].reg_v0 = ret;
}

void read_from_terminal(support_t *s)
{
    int ret = -1;
    // ??
    s->sup_exceptState[GENERALEXCEPT].reg_v0 = ret;
}