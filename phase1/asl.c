#include "asl.h"

/* array di SEMD con dimensione massima di MAXPROC */
semd_PTR semd_table[MAXPROC];
/* Lista dei SEMD liberi */
semdFree_t semdFree_h;
/* Active Semaphore List */
asl_t ASL;

/*
    Viene inserito il PCB puntato da p nella coda dei 
    processi bloccati associata al SEMD con chiave 
    semAdd. Se il semaforo corrispondente non è 
    presente nella ASL, alloca un nuovo SEMD dalla 
    lista di quelli liberi (semdFree) e lo inserisce nella 
    ASL, settando I campi in maniera opportuna (i.e. 
    key e s_procQ). Se non è possibile allocare un 
    nuovo SEMD perché la lista di quelli liberi è vuota, 
    restituisce TRUE. In tutti gli altri casi, restituisce FALSE.
*/
int insertBlocked(int *semAdd, pcb_t *p) {

}