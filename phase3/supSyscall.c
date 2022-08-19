#include "headers/supSyscall.h"

extern int master_sem;
extern void free_sd(support_t *s);

cpu_t get_tod()
{
    cpu_t tod;
    STCK(tod);
    return tod;
}

void terminate(support_t *s)
{
    // marcare la memoria come non occupata
    free_sd(s);
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
}

void read_from_terminal(support_t *s)
{
}