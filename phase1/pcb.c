#include "pcb.h"
#include "listx.h"

//array di PCB con dimensione massima di MAXPROC
pcb_t pcbFree_table[MAXPROC];
struct list_head *pcbFree_h;

void initPcbs(void){
  //pcbFree_h = LIST_HEAD_INIT(pcbFree_h);
  for(int i=0; i < MAXPROC; i++){
		pcb_t* pcb = &pcbFree_table[i];
		list_add_tail(&pcb->p_list, &pcbFree_h);
	}
}

void freePcb(pcb_t * p){
	list_add(&p, &pcbFree_h);
}

void *my_memset(void *s, int c,  unsigned int len) {
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
	//head = LIST_HEAD_INIT(head);
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

pcb_t* removeProcQ(struct list_head* head){
    if(list_empty(&head)) return NULL;
    pcb_t* removedElement = head->next;
    head->next = head->next->next;
    return removedElement;
}

pcb_t* outProcQ(struct list_head* head, pcb_t* p){
    struct list_head* iter;
    list_for_each(iter,head) {
        if(strcmp(&iter,&p)==0){
            list_del(&p);
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
    list_add(&p,&prnt->p_child);
}

pcb_t* removeChild(pcb_t *p){
    if(list_empty(&p->p_child)) return NULL;
    pcb_t* removedElement = p->p_child.next;
    list_del(&(p->p_child.next));
    return removedElement;
}

pcb_t* outChild(pcb_t *p){
    if(list_empty(&p->p_parent)) return NULL;
    list_del(&p);
    return p;
}