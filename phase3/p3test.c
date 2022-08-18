#include "../generic_headers/pandos_const.h"
#include "../generic_headers/pandos_types.h"
#include <umps/libumps.h>
#include "./headers/supVM.h"

// table of usable support descriptor
static support_t sd_table[UPROCMAX];

/*
La funzione test dovra’:
- Inizializzare le strutture dati di fase 3
- Caricare e far partire l’esecuzione dei
processi che vi forniamo
- Mettersi in attesa della fine di questi ultimi
(e’ opportuno che il sistema si fermi una
volta terminati tutti)

Sezione 4.9
*/

void test() {
    init_sup_struct();
    run_test();
    SYSCALL(TERMPROCESS, 0, 0, 0);
}

void init_sup_struct() {
    init_swap_pool_table();

}

void run_test() {

    state_t proc_state;

    pstate.pc_epc = pstate.reg_t9 = (memaddr)UPROCSTARTADDR;
    proc_state.reg_sp = (memaddr)USERSTACKTOP;
    pstate.status = ALLOFF | IEPON | IMON | TEBITON;


    for (int i = 0; i < UPROCMAX; i++) {
        int asid = i+1;
        pstate.entry_hi = asid << ASIDSHIFT;
        
        support_t *s = sd_table[i];
        s->sup_asid = asid;

        init_page_table(s->sup_privatePgTbl, asid);

        // cose

        SYSCALL(CREATEPROCESS, proc_state PROCESS_PRIO_LOW, (int)s);
    }
    
}