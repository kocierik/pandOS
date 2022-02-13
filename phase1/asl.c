#include "asl.h"

/* array di SEMD con dimensione massima di MAXPROC */
semd_PTR semd_table[MAXPROC];
/* Lista dei SEMD liberi */
semd_PTR semdFree_h;
/* Active Semaphore List */
semd_PTR ASL_h;

/*
semd_PTR findASL(int *semAdd) {
    int found = 0;     //sem found flag
    semd_t tmp = *ASL_h;

    //cerco il semaforo
    do{
        if(tmp.s_key == semAdd) found = 1;
        else tmp = *tmp.s_link.next;
    } while (!found && &tmp != ASL_h);

    if(found)
        return &tmp;
    else
        return NULL;
}


int insertBlocked(int *semAdd, pcb_t *p) {
    semd_PTR tmp = findASL(semAdd);

    //se trovato
    if(tmp != NULL) list_add_tail(p,&(*tmp).s_procq); //aggiungo alla coda dei processi bloccati p
    //se non trovato
    else {
        // se non ci sono semafori liberi
        if(list_empty(&semdFree_h->s_link))
            return 1; //return TRUE

        semd_PTR smd = semdFree_h; //prendo il primo semaforo libero
        list_del(&semdFree_h->s_link);          //tolgo il primo semaforo da quelli liberi
        //setto le variabili
        smd->s_key = semAdd;
        INIT_LIST_HEAD(&smd->s_procq);
        list_add_tail(p, &smd->s_procq);   //inserisco il processo bloccato
        p->p_semAdd = semAdd;
        INIT_LIST_HEAD(&smd->s_link);

        asl_PTR newasl = malloc(sizeof(asl_PTR));
        newasl->elem = (*smd);
        INIT_LIST_HEAD(&newasl->ptr_list);
        
        list_add_tail(newasl, &(*ASL).ptr_list);  //aggiungo il semafoto alla lista di quelli attivi
    }
    return 0; //FALSE
}



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


pcb_t* outBlocked(pcb_t *p) {

}


pcb_t* headBlocked(int *semAdd) {

}
*/

//copiata dal pcb, obv cambiando le cose giuste
void initASL() {
    //semdFree_h = LIST_HEAD_INIT(semdFree_h);
    for(int i=0; i < MAXPROC; i++){
		semd_t* semd = &semd_table[i];
		list_add_tail(&semd, &semdFree_h->s_link);
	}
}