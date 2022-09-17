#include "./headers/p3test.h"

// the program routine strarts here
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
    init_sem();
}

// init free support descriptor list
void init_sd_free()
{
    INIT_LIST_HEAD(&sd_free);
    for (int i = 0; i < UPROCMAX; i++)
        free_sd(&sd_table[i]);
}

// init phase 3 device semaphores
void init_sem()
{
    for (int i = 0; i < 8; i++)
        semPrinter_phase3[i] = semTermRead_phase3[i] = semTermWrite_phase3[i] = 1;
}

// return a support descriptor taken from the free sd list
support_t *alloc_sd()
{
    struct list_head *l = list_next(&sd_free);
    list_del(l);
    return container_of(l, support_t, p_list);
}

// add a support desc to the free sd list
void free_sd(support_t *s)
{
    list_add(&s->p_list, &sd_free);
}

// init a proc
void create_uproc(int asid)
{
    memaddr ramaddrs;
    RAMTOP(ramaddrs);

    state_t proc_state;

    proc_state.pc_epc = proc_state.reg_t9 = (memaddr)UPROCSTARTADDR;
    proc_state.reg_sp = (memaddr)USERSTACKTOP;
    proc_state.status = ALLOFF | USERPON | IEPON | IMON | TEBITON; // da mettere? USERPON
    proc_state.entry_hi = asid << ASIDSHIFT;

    support_t *s = alloc_sd();
    s->sup_asid = asid;

    s->sup_exceptContext[PGFAULTEXCEPT].pc = (memaddr)pager;
    s->sup_exceptContext[PGFAULTEXCEPT].stackPtr = ramaddrs - (2 * asid * PAGESIZE);
    s->sup_exceptContext[PGFAULTEXCEPT].status = ALLOFF | IEPON | IMON | TEBITON;

    s->sup_exceptContext[GENERALEXCEPT].pc = (memaddr)general_execption_handler;
    s->sup_exceptContext[GENERALEXCEPT].stackPtr = ramaddrs - (2 * asid * PAGESIZE) + PAGESIZE;
    s->sup_exceptContext[GENERALEXCEPT].status = ALLOFF | IEPON | IMON | TEBITON;

    init_page_table(s->sup_privatePgTbl, asid);

    klog_print("asid ");
    klog_print_dec(asid);
    klog_print(" ");
    bp();

    SYSCALL(CREATEPROCESS, (int)&proc_state, PROCESS_PRIO_LOW, (int)s); // process starts
}

// run every proc
void run_proc()
{
    for (int i = 1; i <= UPROCMAX; i++)
        create_uproc(i); // asid from 1 to 8
    // myprint("all proc loaded\n");

    // wait for others process to end
    for (int i = 1; i <= UPROCMAX; i++) // DA MODIFICARE TODO
        SYSCALL(PASSEREN, (int)&master_sem, 0, 0);
    myprint("Passeren eseguite\n");
}