#include "pcb.h"
#include "listx.h"

//array di PCB con dimensione massima di MAXPROC
pcb_t pcbFree_table[MAXPROC];
//struct list_head *pcbFree_h;
LIST_HEAD(pcbFree_h);

void initPcbs(void){
    for(int i=0; i < MAXPROC; i++){
		pcb_t* pcb = &pcbFree_table[i];
		list_add_tail(&pcb->p_list, &pcbFree_h);
	}
}

void freePcb(pcb_t *p){
	list_add(&p->p_list, &pcbFree_h);
}

pcb_t *allocPcb(){
	if(list_empty(&pcbFree_h)) return NULL;
	else {
		pcb_t *savedElement = container_of(pcbFree_h.next,pcb_t,p_list);
		pcbFree_h.next = pcbFree_h.next->next;

		INIT_LIST_HEAD(&savedElement->p_list);

		savedElement->p_parent = NULL;
		INIT_LIST_HEAD(&savedElement->p_child);
		INIT_LIST_HEAD(&savedElement->p_sib);

		savedElement->p_time = 0;
		savedElement->p_semAdd = NULL;
		return savedElement;
	}
}

void mkEmptyProcQ(struct list_head *head){
	INIT_LIST_HEAD(head);
}

int emptyProcQ(struct list_head *head){
	return list_empty(head) ? 1 : 0;
}

void insertProcQ(struct list_head* head, pcb_t* p){
	list_add_tail(&p->p_list,head);
}
//forse buona
pcb_t* headProcQ(struct list_head* head){
	return list_empty(head) ? NULL : container_of(head->next,pcb_t,p_list);
}

pcb_t* removeProcQ(struct list_head* head){
    if(list_empty(head)) return NULL;
    struct list_head* removedElement = head->next;
    head->next = head->next->next;
    return container_of(removedElement,pcb_t,p_list);
}
//forse buona
pcb_t* outProcQ(struct list_head* head, pcb_t* p){
    struct list_head* iter;
    list_for_each(iter,head) {
        if(container_of(iter,pcb_t,p_list) == p){
            //list_del(&p->p_list);
            list_del(iter);
            return p;
        }
    }
    return NULL;
}

//funzioni per alberi pcb

int emptyChild(pcb_t *p){
    return list_empty(&p->p_child) ? 1 : 0;
}

void insertChild(pcb_t *prnt, pcb_t *p){
    list_add(&p->p_list,&prnt->p_child);
}
//forse buono
pcb_t* removeChild(pcb_t *p){
    if(list_empty(&p->p_child)) return NULL;
    struct list_head *removedElement = p->p_child.next;
    list_del(p->p_child.next);
    return container_of(removedElement,pcb_t,p_child);
}

pcb_t* outChild(pcb_t *p){
    if(p->p_parent != NULL) {
        struct list_head* iter;
        list_for_each(iter,&p->p_parent->p_child) {
            if(container_of(iter,pcb_t,p_list) == p){
                //list_del(&p->p_list);
                list_del(iter);
                return p;
            }
        }
    }
    return NULL;
}