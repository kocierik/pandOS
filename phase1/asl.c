#include "asl.h"

/* array di SEMD con dimensione massima di MAXPROC */
semd_PTR semd_table[MAXPROC];
/* Lista dei SEMD liberi */
semdFree_t semdFree_h;
/* Active Semaphore List */
asl_PTR ASL;


asl_PTR findASL(int *semAdd) {
    int found = 0;     //sem found flag
    asl_PTR tmp = ASL; //semaforo a cui aggiungere il pcb

    //cerco il semaforo
    do{
        if((*tmp).elem.s_key == semAdd) found = 1;
        else tmp = (*tmp).ptr_list.next;
    } while (!found && tmp != ASL);
    if(found)
        return tmp;
    else
        return NULL;
}


int insertBlocked(int *semAdd, pcb_t *p) {
    asl_PTR tmp = findASL(semAdd);

    //se trovato
    if(tmp != NULL) list_add_tail(p,&(*tmp).elem.s_procq); //aggiungo alla coda dei processi bloccati p
    //se non trovato
    else {
        // se non ci sono semafori liberi
        if(list_empty(&semdFree_h.ptr_list))
            return 1; //return TRUE

        semd_t* smd = &semdFree_h.elem; //prendo il primo semaforo libero
        list_del(&semdFree_h);          //tolgo il primo semaforo da quelli liberi
        //setto le variabili
        smd->s_key = semAdd;
        INIT_LIST_HEAD(&smd->s_procq);
        list_add_tail(p, &smd->s_procq);   //inserisco il processo bloccato
        INIT_LIST_HEAD(&smd->s_link);

        asl_PTR newasl = malloc(sizeof(asl_PTR));
        newasl->elem = (*smd);
        INIT_LIST_HEAD(&newasl->ptr_list);
        
        list_add_tail(newasl, &(*ASL).ptr_list);  //aggiungo il semafoto alla lista di quelli attivi
    }
    return 0; //FALSE
}


/*
    Ritorna il primo PCB dalla coda dei processi 
    bloccati (s_procq) associata al SEMD della 
    ASL con chiave semAdd. Se tale descrittore 
    non esiste nella ASL, restituisce NULL. 
    Altrimenti, restituisce l’elemento rimosso. Se 
    la coda dei processi bloccati per il semaforo 
    diventa vuota, rimuove il descrittore 
    corrispondente dalla ASL e lo inserisce nella 
    coda dei descrittori liberi (semdFree_h).
*/
pcb_t* removeBlocked(int *semAdd) {
    asl_PTR tmp = findASL(semAdd); //trovo il semaforo giusto

    if (tmp != NULL) {
        pcb_t *val = (*tmp).elem.s_procq.next;  //prendo il primo pcb
        list_del((*tmp).elem.s_procq.next);
        if(list_empty(&(*tmp).elem.s_procq)) {  //se la lista dei pcb è vuota libero il semaforo

        }
        return val;
    }
    else return NULL;
}


/*
    Rimuove il PCB puntato da p dalla coda del semaforo 
    su cui è bloccato (indicato da p->p_semAdd). Se il PCB 
    non compare in tale coda, allora restituisce NULL 
    (*condizione di errore*). Altrimenti, restituisce p. Se la 
    coda dei processi bloccati per il semaforo diventa 
    vuota, rimuove il descrittore corrispondente dalla ASL 
    e lo inserisce nella coda dei descrittori liberi.
*/
pcb_t* outBlocked(pcb_t *p) {

}


/*
    Restituisce (senza rimuovere) il puntatore al PCB che 
    si trova in testa alla coda dei processi associata al 
    SEMD con chiave semAdd. Ritorna NULL se il SEMD 
    non compare nella ASL oppure se compare ma la sua 
    coda dei processi è vuota.
*/
pcb_t* headBlocked(int *semAdd) {

}



//copiata dal pcb, obv cambiando le cose giuste
void initASL() {
    for(int i = 0; i < MAXPROC; i++) {
        semd_t* smd = &semd_table[i];
        list_add_tail(&(smd->s_procq.next),&(semdFree_h));
    }
}