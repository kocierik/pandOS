#include "asl.h"

static semd_t semd_table[MAXPROC];     /* SEMD array with maximum size 'MAXPROC' */
static LIST_HEAD(semdFree_h);          /* List of free SEMD */
static LIST_HEAD(ASL_h);               /* Active Semaphore List */


semd_PTR findASL(int *semAdd) {
    semd_PTR sem;
    struct list_head *pos;

    // Looking for Semaphore
    list_for_each(pos,&ASL_h){
        sem = container_of(pos, semd_t, s_link);
        if(sem->s_key == semAdd)
            return sem;
    }
    return NULL;
}


int isSemdFree(semd_PTR sem) {
    // Free the Semaphore if pcb's list is empty
    if(list_empty(&sem->s_procq)) {
        list_del(&sem->s_link);
        list_add_tail(&sem->s_link, &semdFree_h);
        return 1;
    }
    return 0;
}


int insertBlocked(int *semAdd, pcb_t *p) {
    semd_PTR sem = findASL(semAdd);

    // If p found add p to blocked process queue
    if(sem != NULL){
        p->p_semAdd = semAdd;
        insertProcQ(&sem->s_procq, p);
    } else {
        if(list_empty(&semdFree_h))
            return 1;                                           // If there are no free Semaphore return TRUE

        sem = container_of(semdFree_h.next, semd_t, s_link);    // Take the first free Semaphore
        list_del(&sem->s_link);                                 // Remove the first Semaphore from the free ones
        
        // initialize variables
        p->p_semAdd = semAdd;
        sem->s_key  = semAdd;
        INIT_LIST_HEAD(&sem->s_procq);
        insertProcQ(&sem->s_procq, p);                          // Insert blocked process
        list_add_tail(&sem->s_link, &ASL_h);                    // Add Semaphore to the active ones list
    }
    return 0; //FALSE
}


pcb_t* removeBlocked(int *semAdd) {
    semd_PTR sem = findASL(semAdd);
    pcb_PTR ret;

    if (sem != NULL && !isSemdFree(sem)) {
        ret = container_of(sem->s_procq.next, pcb_t, p_list);   // Take the first PCB
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