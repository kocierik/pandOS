#include "asl.h"

/* array di SEMD con dimensione massima di MAXPROC */
semd_PTR semd_table[MAXPROC];
/* Lista dei SEMD liberi */
semd_PTR semdFree_h;
/* Active Semaphore List */
semd_PTR ASL_h;


semd_PTR findASL(int semAdd) {
    int found = 0;     //sem found flag
    semd_PTR tmp = ASL_h;

    //cerco il semaforo
    do{
        if(tmp->s_key == semAdd) found = 1;
        else tmp = &(*tmp).s_link.next;
    } while (!found && &tmp != ASL_h);

    if(found)
        return &tmp;
    else
        return NULL;
}


int insertBlocked(int *semAdd, pcb_t *p) {
    semd_PTR tmp = findASL(*semAdd);

    //se trovato
    if(tmp != NULL) list_add_tail(p,&(*tmp).s_procq); //aggiungo alla coda dei processi bloccati p
    //se non trovato
    else {
        // se non ci sono semafori liberi
        if(list_empty(&semdFree_h->s_link))
            return 1; //return TRUE

        semd_PTR smd = semdFree_h;      //prendo il primo semaforo libero
        list_del(&semdFree_h->s_link);  //tolgo il primo semaforo da quelli liberi
        //setto le variabili
        smd->s_key = semAdd;
        INIT_LIST_HEAD(&smd->s_procq);
        list_add_tail(p, &smd->s_procq);   //inserisco il processo bloccato
        p->p_semAdd = semAdd;
        INIT_LIST_HEAD(&smd->s_link);

        list_add_tail(smd, &ASL_h->s_link);  //aggiungo il semafoto alla lista di quelli attivi
    }
    return 0; //FALSE
}



pcb_t* removeBlocked(int *semAdd) {
    semd_PTR tmp = findASL(*semAdd); //trovo il semaforo giusto

    if (tmp != NULL) {
        pcb_t *val = (*tmp).s_procq.next;  //prendo il primo pcb
        list_del((*tmp).s_procq.next);
        if(list_empty(&(*tmp).s_procq)) {  //se la lista dei pcb è vuota libero il semaforo
            list_del(&tmp->s_link);
            list_add_tail(tmp, &semdFree_h->s_link);
        }
        return val;
    }
    else return NULL;
}


pcb_t* outBlocked(pcb_t *p) {
    int semAdd = p->p_semAdd;
    semd_PTR tmp = findASL(semAdd);
    
    if(tmp != NULL){
        int found = 0;
        pcb_PTR ptmp = &tmp->s_procq;

        do{
            if(ptmp == p) found = 1;
            else ptmp = &(*tmp).s_procq.next;
        } while (!found && ptmp != &tmp->s_procq);

        if (found)  return p;
        else        return NULL;
    }
    else return NULL;
}


pcb_t* headBlocked(int *semAdd) {
    semd_PTR tmp = findASL(*semAdd);
    return &tmp->s_procq;
}


//copiata dal pcb, obv cambiando le cose giuste
void initASL() {
    //semdFree_h = LIST_HEAD_INIT(semdFree_h);
    for(int i=0; i < MAXPROC; i++){
		semd_t* semd = &semd_table[i];
		list_add_tail(&semd, &semdFree_h->s_link);
	}
}