#include "asl.h"

/* array di SEMD con dimensione massima di MAXPROC */
semd_PTR semd_table[MAXPROC];
/* Lista dei SEMD liberi */
struct list_head semdFree_h;
/* Active Semaphore List */
struct list_head ASL_h;


semd_PTR findASL(int *semAdd) {

    if(list_empty(&ASL_h))
        return NULL;     // se ASL è vuota -> errore

    int found = 0;                          //sem found flag
    struct list_head *tmp = ASL_h.next;    //assumo che l'head non appartenga a nessun sem
    semd_PTR sem;

    //cerco il semaforo
    while (!found && tmp != &ASL_h) {
        sem = container_of(tmp, struct semd_t, s_link);

        if(sem->s_key == semAdd)
            found = 1;
        tmp = tmp->next;
    }

    if(found)
        return sem;
    return NULL;
}


int insertBlocked(int *semAdd, pcb_t *p) {
    semd_PTR sem, tmp = findASL(semAdd);

    //se trovato aggiungo alla coda dei processi bloccati p
    if(tmp != NULL) list_add_tail(&p->p_list, &tmp->s_procq);
    else {
        // se non ci sono semafori liberi
        if(list_empty(&semdFree_h))
            return 1; //TRUE

        sem = container_of(semdFree_h.next, struct semd_t, s_link);    // prendo il primo semaforo libero
        list_del(semdFree_h.next);                              // tolgo il primo semaforo da quelli liberi
        
        // inizializzo le variabili
        sem->s_key = semAdd;
        p->p_semAdd = semAdd;
        list_add_tail(&p->p_list, &sem->s_procq);   //inserisco il processo bloccato

        // DA GUARDARE MEGLIO
        // serve inizializzarle queste liste? come?
        //LIST_HEAD(sem->s_procq);
        //LIST_HEAD(sem->s_link);
        //sem->s_procq = LIST_HEAD_INIT(*sem.s_procq);
        //sem->s_link = LIST_HEAD_INIT(*sem.s_link);

        INIT_LIST_HEAD(&sem->s_procq);
        INIT_LIST_HEAD(&sem->s_link);

        list_add_tail(&sem->s_link, &ASL_h);  //aggiungo il semaforo alla lista di quelli attivi
    }
    return 0; //FALSE
}



pcb_t* removeBlocked(int *semAdd) {
    semd_PTR sem = findASL(semAdd); //trovo il semaforo giusto
    pcb_PTR ret;

    if (sem != NULL) {
        ret = container_of(sem->s_procq.next, struct pcb_t, p_list);    // prendo il primo pcb
        list_del(sem->s_procq.next);
        if(list_empty(&sem->s_procq)) {  //se la lista dei pcb è vuota libero il semaforo
            list_del(&sem->s_link);
            list_add_tail(&sem->s_link, &semdFree_h);
        }
        return ret;
    }
    return NULL;
}



pcb_t* outBlocked(pcb_t *p) {
    int *semAdd = p->p_semAdd;
    semd_PTR tmp = findASL(semAdd);
    
    if(tmp != NULL){
        int found = 0;
        struct list_head * ptmp = tmp->s_procq.next;

        do{
            pcb_PTR pList = container_of(ptmp, struct pcb_t, p_list);
            if(pList == p) found = 1;
            else ptmp = ptmp->next;
        } while (!found && ptmp != &tmp->s_procq);

        if (found) {
            list_del(ptmp);
            if(list_empty(&(*tmp).s_procq)) {  //se la lista dei pcb è vuota libero il semaforo
                list_del(&tmp->s_link);
                list_add_tail(&tmp->s_link, &semdFree_h);
            }
            return p;
        }
        else return NULL;
    }
    else return NULL;
}


pcb_t* headBlocked(int *semAdd) {
    semd_PTR sem = findASL(semAdd);
    
    if(sem != NULL && !list_empty(&sem->s_procq))
        return container_of(sem->s_procq.next, struct pcb_t, p_list);

    return NULL;
}


//copiata dal pcb, obv cambiando le cose giuste
void initASL() {
    LIST_HEAD(ASL_h);
    LIST_HEAD(semdFree_h);
    INIT_LIST_HEAD(&ASL_h);
    INIT_LIST_HEAD(&semdFree_h);
    for(int i=0; i < MAXPROC; i++){
		semd_t * semd = semd_table[i];
		list_add_tail(&semd->s_link, &semdFree_h);
	}
}