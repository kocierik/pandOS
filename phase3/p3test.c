#include "../generic_headers/pandos_const.h"
#include "../generic_headers/pandos_types.h"
#include <umps/libumps.h>
#include "./headers/VMSupport.h"

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
    initSwap();
    initPageTable();
    initSem();
}

void run_test() {

}