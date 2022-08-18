#include "../generic_headers/pandos_const.h"
#include "../generic_headers/pandos_types.h"
#include <umps/libumps.h>
#include "./headers/VMSupport.h"

// table of usable support descriptor
static support_t sd_table[UPROCMAX];
// list of free support descriptor
static list_head sd_free;

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
    INIT_LIST_HEAD(&sd_free);
    for (int i = 0; i < UPROCMAX; i++)
        free_sd(sd_table[i]);
}

// return a support descriptor taken from the free sd list
support_t alloc_sd(){
    list_head *l = list_next(&sd_free);
    list_del(l);
    return container_of(l, support_t, p_list);
}

// add a support descriptor to the free sd list
void free_sd(support_t *s) {
    list_add(&s->p_list, &sd_free);
}

void run_test() {

}