#include "../generic_headers/pandos_const.h"
#include "../generic_headers/pandos_types.h"
#include <umps/libumps.h>
#include "./headers/VMSupport.h"


static support_t sd_table[UPROCMAX];    // table of usable support descriptor
static list_head sd_free;               // list of free support descriptor

/*
La funzione test dovra’:
- Inizializzare le strutture dati di fase 3
- Caricare e far partire l’esecuzione dei
processi che vi forniamo
- Mettersi in attesa della fine di questi ultimi
(e’ opportuno che il sistema si fermi una
volta terminati tutti)
*/

void test() {
    init_sup_struct();
    run_test();
    SYSCALL(TERMPROCESS,0,0,0);
}

void init_sup_struct() {
    init_sd_free()

}

//init free support descriptor list
void init_sd_free(){
    INIT_LIST_HEAD(&support_free);
    for (int i = 0; i < UPROCMAX; i++) {
        list_head *l = &(sd_table[i])->p_list;
        list_add(l, &support_free);
    }
}


void run_test() {

}