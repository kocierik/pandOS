#include "asl.h"

/* array di SEMD con dimensione massima di MAXPROC */
semd_t semd_table[MAXPROC];
/* Lista dei SEMD liberi */
LIST_HEAD(semdFree_h);
/* Active Semaphore List */
LIST_HEAD(ASL_h);


semd_PTR findASL(int *semAdd) {
    semd_PTR sem;
    struct list_head *pos;

    //cerco il semaforo
    list_for_each(pos,&ASL_h){
        sem = container_of(pos, semd_t, s_link);
        if(sem->s_key == semAdd)
            return sem;
    }
    return NULL;
}



void freeSem(semd_PTR sem) {
    // se la lista dei pcb e' vuota libero il semaforo
    if(list_empty(&sem->s_procq)) {
        list_del(&sem->s_link);
        sem->s_key = NULL;
        list_add_tail(&sem->s_link, &semdFree_h);
    }
}



int insertBlocked(int *semAdd, pcb_t *p) {
    semd_PTR sem = findASL(semAdd);

    //se trovato aggiungo alla coda dei processi bloccati p
    if(sem != NULL)
        insertProcQ(&sem->s_procq, p);
    else {
        if(list_empty(&semdFree_h))
            return 1;                                           // se non ci sono semafori liberi ritorna TRUE

        sem = container_of(semdFree_h.next, semd_t, s_link);    // prendo il primo semaforo libero
        list_del(&sem->s_link);                                 // tolgo il primo semaforo da quelli liberi
        
        // inizializzo le variabili
        sem->s_key  = semAdd;
        p->p_semAdd = semAdd;
        INIT_LIST_HEAD(&sem->s_procq);
        insertProcQ(&sem->s_procq, p);                          // inserisco il processo bloccato
        list_add_tail(&sem->s_link, &ASL_h);                    // aggiungo il semaforo alla lista di quelli attivi
    }
    return 0; //FALSE
}



pcb_t* removeBlocked(int *semAdd) {
    semd_PTR sem = findASL(semAdd);
    pcb_PTR ret;

    if (sem != NULL) {
        if(list_empty(&sem->s_procq)){
            freeSem(sem);
            return NULL;
        }
        ret = container_of(sem->s_procq.next, pcb_t, p_list);   // prendo il primo pcb
        list_del(sem->s_procq.next);
        freeSem(sem);
        ret->p_semAdd = NULL;
        return ret;
    }
    return NULL;
}


/*
pcb_t* outBlocked(pcb_t *p) {
    semd_PTR sem = findASL(p->p_semAdd);
    
    if(sem != NULL){
        pcb_PTR pList;
        struct list_head *pos;

        p->p_semAdd = NULL;
        // cerco il pcb
        list_for_each(pos, &sem->s_procq){
            pList = container_of(pos, pcb_t, p_list);
            if(p == pList){
                list_del(pos);
                freeSem(sem);
                return p;
            }
        }
    }
    return NULL;
}
*/
pcb_t* outBlocked(pcb_t *p) {
    if(p->p_semAdd != NULL){
        semd_PTR sem = findASL(p->p_semAdd);
        
        if(sem != NULL){
            if(sem->s_procq.next == &p->p_list)
                return removeBlocked(p->p_semAdd);
            
            pcb_PTR pList;
            struct list_head *pos;

            p->p_semAdd = NULL;

            list_for_each(pos, &sem->s_procq){
                pList = container_of(pos, pcb_t, p_list);
                if(p == pList){
                    list_del(&p->p_list);
                    freeSem(sem);
                    return p;
                }
            }
        }
    }
    return NULL;
}



pcb_t* headBlocked(int *semAdd) {
    semd_PTR sem = findASL(semAdd);
    
    if(sem != NULL){
        if(!list_empty(&sem->s_procq))
            return container_of(sem->s_procq.next, pcb_t, p_list);
        else
            freeSem(sem);
    }
    return NULL;
}



void initASL() {
    for(int i=0; i < MAXPROC; i++){
		semd_t* semd = &semd_table[i];
		list_add_tail(&semd->s_link, &semdFree_h);
	}
}