#include "./headers/p3test.h"

void test()
{
    init_sds();
    run_proc();
    SYSCALL(TERMPROCESS, 0, 0, 0); // HALT
}

// init support data structures
void init_sds()
{
    master_sem = 0;
    init_swap_pool_table();
    init_sd_free();
}

// init free support descriptor list
void init_sd_free()
{
    INIT_LIST_HEAD(&sd_free);
    for (int i = 0; i < UPROCMAX; i++)
        free_sd(&sd_table[i]);
}

// return a support descriptor taken from the free sd list
support_t *alloc_sd()
{
    struct list_head *l = list_next(&sd_free);
    list_del(l);
    return container_of(l, support_t, p_list);
}

// add a support descriptor to the free sd list
void free_sd(support_t *s)
{
    list_add(&s->p_list, &sd_free);
}

// init an uproc
void create_uproc(int asid)
{
    memaddr ramaddrs;
    state_t proc_state;

    RAMTOP(ramaddrs);

    proc_state.entry_hi = asid << ASIDSHIFT;
    proc_state.pc_epc = proc_state.reg_t9 = UPROCSTARTADDR;
    proc_state.reg_sp = USERSTACKTOP;
    proc_state.status = ALLOFF | USERPON | IEPON | IMON | TEBITON;

    support_t *s = alloc_sd();
    s->sup_asid = asid;

    s->sup_exceptContext[PGFAULTEXCEPT].status = ALLOFF | IEPON | IMON | TEBITON;
    s->sup_exceptContext[PGFAULTEXCEPT].pc = (memaddr)pager;
    s->sup_exceptContext[PGFAULTEXCEPT].stackPtr = ramaddrs - (2 * asid * PAGESIZE) + PAGESIZE;

    s->sup_exceptContext[GENERALEXCEPT].status = ALLOFF | IEPON | IMON | TEBITON;
    s->sup_exceptContext[GENERALEXCEPT].pc = (memaddr)general_execption_handler;
    s->sup_exceptContext[GENERALEXCEPT].stackPtr = ramaddrs - (2 * asid * PAGESIZE);

    init_page_table(s->sup_privatePgTbl, asid);

    SYSCALL(CREATEPROCESS, (int)&proc_state, PROCESS_PRIO_LOW, (int)s); // process starts
}

// run every proc
void run_proc()
{
    for (int i = 0; i < UPROCMAX; i++)
        create_uproc(i + 1); // asid from 1 to 8

    // wait for others process to end
    for (int i = 0; i < UPROCMAX; i++)
        SYSCALL(PASSEREN, (int)&master_sem, 0, 0);
}