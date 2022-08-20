#include "./headers/p3test.h"

// Sezione 4.9

// table of usable support descriptor
static support_t sd_table[UPROCMAX];
// list of free support descriptor
static struct list_head sd_free;

void test()
{
    init_sds();
    run_test();
    SYSCALL(TERMPROCESS, 0, 0, 0);
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

void run_test()
{
    memaddr ramaddrs;
    state_t proc_state;

    proc_state.pc_epc = proc_state.reg_t9 = (memaddr)UPROCSTARTADDR;
    proc_state.reg_sp = (memaddr)USERSTACKTOP;
    proc_state.status = ALLOFF | IEPON | IMON | TEBITON;

    RAMTOP(ramaddrs);

    for (int i = 0; i < UPROCMAX; i++)
    {
        int asid = i + 1; // unique asid from 1 to 8
        proc_state.entry_hi = asid << ASIDSHIFT;

        support_t *s = alloc_sd();
        s->sup_asid = asid;

        init_page_table(s->sup_privatePgTbl, asid);

        // cose

        SYSCALL(CREATEPROCESS, (int)&proc_state, PROCESS_PRIO_LOW, (int)s); // process starts
    }

    // wait others process to end before exit
    for (int i = 0; i < UPROCMAX; i++)
        SYSCALL(PASSEREN, (int)&master_sem, 0, 0);
}