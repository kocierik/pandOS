#include "pcb.h"
#include "listx.h"

//array di PCB con dimensione massima di MAXPROC
pcb_t pcbFree_table[MAXPROC];
LIST_HEAD(pcbFree_h);

void initPcbs(void){
  for(int i=0; i < MAXPROC; i++){
		pcb_t* pcb = &pcbFree_table[i];
		list_add_tail(&pcb->p_list, &pcbFree_h);
	}
}

void freePcb(pcb_t *p){
	list_add_tail(&p->p_list, &pcbFree_h);
}

pcb_t *allocPcb(){
	if(list_empty(&pcbFree_h)) return NULL;
	else {
		pcb_t *savedElement = container_of(pcbFree_h.next,pcb_t,p_list);
        list_del(pcbFree_h.next);

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

pcb_t* headProcQ(struct list_head* head){
	return list_empty(head) ? NULL : container_of(head->next,pcb_t,p_list);
}

pcb_t* removeProcQ(struct list_head* head){
    if(list_empty(head)) return NULL;
    struct list_head* removedElement = head->next;
    //head->next = head->next->next;
    list_del(head->next);
    return container_of(removedElement,pcb_t,p_list);
}

pcb_t* outProcQ(struct list_head* head, pcb_t* p){
    struct list_head* pos;
    list_for_each(pos,head) {
        if(container_of(pos,pcb_t,p_list) == p){
            list_del(&p->p_list);
            //list_del(pos);
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
    //add p to prnt children list
    list_add_tail(&p->p_list,&prnt->p_child);
    //prnt is the parent of p
    p->p_parent = prnt;
    //add p to p_sib list that starts from prnt's first child
    pcb_t *firstChild = container_of(prnt->p_child.next, pcb_t, p_child);
    list_add_tail(&p->p_sib, &firstChild->p_sib);
    // struct list_head *pos;
    // list_for_each(pos,&prnt->p_child){
    //     if(list_is_last(pos->next,&prnt->p_child)){
    //         pcb_t *last_sib = container_of(&prnt->p_child,pcb_t,p_child);
    //         list_add_tail(&p->p_sib,&last_sib->p_sib);
    //     }
    // }
}

pcb_t* removeChild(pcb_t *p){
    if(list_empty(&p->p_child)) return NULL;
    struct pcb_t *firstChild = container_of(p->p_child.next, pcb_t, p_child);
    //remove firstChild from p_sib list
    list_del(&firstChild->p_sib);
    //remove p's first child from p_child list
    list_del(p->p_child.next);
    //firstChild as no parent anymore
    firstChild->p_parent = NULL;
    return firstChild;
}

pcb_t* outChild(pcb_t *p){
    pcb_t* prnt = p->p_parent;
    if(prnt != NULL){
        //case if p prnt's first child
        if(p->p_sib.prev == &p->p_sib){
            return removeChild(prnt);
        }
        //remove p from p_child list
        list_del(&p->p_list);
        //remove p from p_sib list
        list_del(&p->p_sib);
        //p as no parent anymore
        p->p_parent = NULL;
        return p;
    }
    return NULL;
    // if(prnt != NULL) {
    //     list_del(&p->p_sib);
    //     //verifichiamo se il figlio è il primo
    //     if(p->p_sib.prev == &p->p_sib){
    //         return removeChild(prnt);
    //     }
    //     //caso in cui in cui il figlio non sia il primo
    //     else{
    //         struct list_head *pos;
    //         list_for_each(pos,&prnt->p_child){
    //             if(container_of(pos, pcb_t, p_child) == p){
    //                 list_del(pos);
    //                 return p;
    //             }
    //         }
    //     }
    // }
    // return NULL;
}