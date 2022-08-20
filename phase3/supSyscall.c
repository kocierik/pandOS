#include "headers/supSyscall.h"

extern int swap_pool_sem;
extern int master_sem;
extern void free_sd(support_t *s);

void get_tod(support_t *s)
{
    cpu_t tod;
    STCK(tod);
    s->sup_exceptState[GENERALEXCEPT].reg_v0 = tod;
}

void terminate(support_t *s)
{
    // marcare la memoria come non occupata
    free_sd(s);
    // SYSCALL(VERHOGEN, (int)&swap_pool_sem, 0, 0);   // release swap pool semaphore, serve davvero?
    SYSCALL(VERHOGEN, (int)&master_sem, 0, 0);
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
    s->sup_exceptState[GENERALEXCEPT].reg_v0 = ret;
}

void read_from_terminal(support_t *s)
{
    int ret = -1;
    s->sup_exceptState[GENERALEXCEPT].reg_v0 = ret;
}