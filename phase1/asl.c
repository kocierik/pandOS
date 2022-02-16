#include "asl.h"

semd_t semd_table[MAXPROC];     /* array di SEMD con dimensione massima di MAXPROC */
LIST_HEAD(semdFree_h);          /* Lista dei SEMD liberi */
LIST_HEAD(ASL_h);               /* Active Semaphore List */


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


int isSemdFree(semd_PTR sem) {
    // se la lista dei pcb e' vuota libero il semaforo
    if(list_empty(&sem->s_procq)) {
        list_del(&sem->s_link);
        list_add_tail(&sem->s_link, &semdFree_h);
        return 1;
    }
    return 0;
}


int insertBlocked(int *semAdd, pcb_t *p) {
    semd_PTR sem = findASL(semAdd);

    //se trovato aggiungo alla coda dei processi bloccati p
    if(sem != NULL){
        p->p_semAdd = semAdd;
        insertProcQ(&sem->s_procq, p);
    } else {
        if(list_empty(&semdFree_h))
            return 1;                                           // se non ci sono semafori liberi ritorna TRUE

        sem = container_of(semdFree_h.next, semd_t, s_link);    // prendo il primo semaforo libero
        list_del(&sem->s_link);                                 // tolgo il primo semaforo da quelli liberi
        
        // inizializzo le variabili
        p->p_semAdd = semAdd;
        sem->s_key  = semAdd;
        INIT_LIST_HEAD(&sem->s_procq);
        insertProcQ(&sem->s_procq, p);                          // inserisco il processo bloccato
        list_add_tail(&sem->s_link, &ASL_h);                    // aggiungo il semaforo alla lista di quelli attivi
    }
    return 0; //FALSE
}


pcb_t* removeBlocked(int *semAdd) {
    semd_PTR sem = findASL(semAdd);
    pcb_PTR ret;

    if (sem != NULL && !isSemdFree(sem)) {
        ret = container_of(sem->s_procq.next, pcb_t, p_list);   // prendo il primo pcb
        list_del(sem->s_procq.next);
        ret->p_semAdd = NULL;
        isSemdFree(sem);
        return ret;
    }
    return NULL;
}


pcb_t* outBlocked(pcb_t *p) {
    semd_PTR sem;
    pcb_PTR pList;
    struct list_head *pos;

    if(p->p_semAdd != NULL && (sem = findASL(p->p_semAdd)) != NULL){
        p->p_semAdd = NULL;

        list_for_each(pos, &sem->s_procq){
            pList = container_of(pos, pcb_t, p_list);
            if(p == pList){
                list_del(&p->p_list);
                isSemdFree(sem);
                return p;
            }
        }
    }
    return NULL;
}


pcb_t* headBlocked(int *semAdd) {
    semd_PTR sem = findASL(semAdd);
    
    if(sem != NULL && !isSemdFree(sem))
        return container_of(sem->s_procq.next, pcb_t, p_list);
    
    return NULL;
}


void initASL() {
    for(int i=0; i < MAXPROC; i++){
		semd_t* semd = &semd_table[i];
		list_add_tail(&semd->s_link, &semdFree_h);
	}
}