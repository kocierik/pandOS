#include "pcb.h"
#include "listx.h"

//array di PCB con dimensione massima di MAXPROC
pcb_t pcbFree_table[MAXPROC];
struct list_head *pcbFree_h;

void initPcbs(void){
  pcbFree_h = LIST_HEAD_INIT(pcbFree_h);
  for(int i=0; i < MAXPROC; i++){
		pcb_t* pcb = &pcbFree_table[i];
		list_add_tail(&pcb->p_list, &pcbFree_h);
	}
}

void freePcb(pcb_t * p){
	list_add(&p, &pcbFree_h);
}

void *my_memset(void *s, int c,  unsigned int len){
    unsigned char* p=s;
    while(len--)
    {
        *p++ = (unsigned char)c;
    }
    return s;
}


void *my_memset(void *s, int c,  unsigned int len)
{
    unsigned char* p=s;
    while(len--)
    {
        *p++ = (unsigned char)c;
    }
    return s;
}

pcb_t *allocPcb(){
	if(list_empty(&pcbFree_h)) return NULL;
	else {
		pcb_t* savedElement = pcbFree_h->next;
		pcbFree_h->next = pcbFree_h->next->next;

		savedElement->p_list = savedElement->p_list;

		savedElement->p_child = savedElement->p_child;
		savedElement->p_parent = savedElement->p_parent;

		savedElement->p_child = savedElement->p_child;
		savedElement->p_sib = savedElement->p_sib;

		memset(savedElement->p_s, 0, sizeof savedElement->p_s);
		savedElement->p_time = 0;
		savedElement->p_semAdd = NULL;
		return savedElement;
	}
}

void mkEmptyProcQ(struct list_head *head){
	head = LIST_HEAD_INIT(head);
}

int emptyProcQ(struct list_head *head){
	return list_empty(&head) ? 1 : 0;
}

void insertProcQ(struct list_head* head, pcb_t* p){
	list_add_tail(&p,&head);
}

pcb_t* headProcQ(struct list_head* head){
	return list_empty(&head) ? NULL : head->next;
}

/* // ? ordine giusto
    Rimuove il primo elemento dalla coda dei 
    processi puntata da head. Ritorna NULL se la 
    coda è vuota. Altrimenti ritorna il puntatore 
    all’elemento rimosso dalla lista.
*/
pcb_t* removeProcQ(struct list_head* head){
    pcb_t* removedElement = head->next;
    head->next = head->next->next;
    return list_empty(&head) ? NULL : removedElement;
}

/*
    Rimuove il PCB puntato da p dalla coda dei 
    processi puntata da head. Se p non è presente 
    nella coda, restituisce NULL. (NOTA: p può 
    trovarsi in una posizione arbitraria della coda).
*/
pcb_t* outProcQ(struct list_head* head, pcb_t* p){

}


//funzioni per alberi pcb

int emptyChild(pcb_t *p){
    return list_empty(&p->p_child) ? 1 : 0;
}

void insertChild(pcb_t *prnt, pcb_t *p){
    list_add(&p,&prnt->p_child);
}

/* //?
    Rimuove il primo figlio del PCB puntato 
    da p. Se p non ha figli, restituisce NULL.
    Altrimenti ritorna il puntatore all'elemento rimosso
*/
pcb_t* removeChild(pcb_t *p){
    if(list_empty(p->p_child)) return NULL;
    pcb_t* removedElement = p->p_child->next;
    list_del(p->p_child->next);
    return removedElement;
    
}

/*
    Rimuove il PCB puntato da p dalla lista 
    dei figli del padre. Se il PCB puntato da 
    p non ha un padre, restituisce NULL, 
    altrimenti restituisce l’elemento 
    rimosso (cioè p). A differenza della 
    removeChild, p può trovarsi in una 
    posizione arbitraria (ossia non è 
    necessariamente il primo figlio del 
    padre).
*/
pcb_t* outChild(pcb_t *p);