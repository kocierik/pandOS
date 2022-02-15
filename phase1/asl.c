#include "asl.h"

/* array di SEMD con dimensione massima di MAXPROC */
semd_t semd_table[MAXPROC];
/* Lista dei SEMD liberi */
LIST_HEAD(semdFree_h);
/* Active Semaphore List */
LIST_HEAD(ASL_h);


semd_PTR findASL(int *semAdd) {
    int found = 0;
    struct list_head *tmp = ASL_h.next;
    semd_PTR sem = NULL;

    //cerco il semaforo
    while (!found && tmp != &ASL_h) {
        sem = container_of(tmp, semd_t, s_link);
        if(sem->s_key == semAdd)
            found = 1;
        tmp = tmp->next;
    }
    
    if(found)
        return sem;
    return NULL;
}



void freeSem(semd_PTR sem) {
    // se la lista dei pcb è vuota libero il semaforo
    if(list_empty(&sem->s_procq)) {
        list_del(&sem->s_link);
        list_add_tail(&sem->s_link, &semdFree_h);
    }
}



int insertBlocked(int *semAdd, pcb_t *p) {
    semd_PTR sem, tmp = findASL(semAdd);

    //se trovato aggiungo alla coda dei processi bloccati p
    if(tmp != NULL)
        list_add_tail(&p->p_list, &tmp->s_procq);
    else {
        if(list_empty(&semdFree_h))
            return 1;                                           // se non ci sono semafori liberi ritorna TRUE

        sem = container_of(semdFree_h.next, semd_t, s_link);    // prendo il primo semaforo libero
        list_del(&sem->s_link);                                 // tolgo il primo semaforo da quelli liberi
        
        // inizializzo le variabili
        sem->s_key = semAdd;
        p->p_semAdd = semAdd;
        INIT_LIST_HEAD(&sem->s_procq);
        list_add_tail(&p->p_list, &sem->s_procq);               // inserisco il processo bloccato
        list_add_tail(&sem->s_link, &ASL_h);                    // aggiungo il semaforo alla lista di quelli attivi
    }
    return 0; //FALSE
}



pcb_t* removeBlocked(int *semAdd) {
    semd_PTR sem = findASL(semAdd);
    pcb_PTR ret;

    if (sem != NULL) {
        ret = container_of(sem->s_procq.next, pcb_t, p_list);   // prendo il primo pcb
        list_del(sem->s_procq.next);
        freeSem(sem);
        return ret;
    }
    return NULL;
}



pcb_t* outBlocked(pcb_t *p) {
    semd_PTR sem = findASL(p->p_semAdd);
    
    if(sem != NULL){
        int found = 0;
        pcb_PTR pList = NULL;
        struct list_head * ptmp = sem->s_procq.next;

        // cerco il pcb
        while(!found && ptmp != &sem->s_procq) {
            pList = container_of(ptmp, pcb_t, p_list);
            if(p == pList)
                found = 1;
            else
                ptmp = ptmp->next;
        }

        if(found) {
            list_del(ptmp);
            freeSem(sem);
            return p;
        }
    }
    return NULL;
}


pcb_t* headBlocked(int *semAdd) {
    semd_PTR sem = findASL(semAdd);
    
    if(sem != NULL && !list_empty(&sem->s_procq))
        return container_of(sem->s_procq.next, pcb_t, p_list);

    return NULL;
}



void initASL() {
    for(int i=0; i < MAXPROC; i++){
		semd_t* semd = &semd_table[i];
		list_add_tail(&semd->s_link, &semdFree_h);
	}
}