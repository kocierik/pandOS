#include "pcb.h"
#include "listx.h"

//array di PCB con dimensione massima di MAXPROC
pcb_t pcbFree_table[MAXPROC];
struct list_head pcbFree_h;

void initPcbs(void){
  pcbFree_h = LIST_HEAD_INIT(&pcbFree_h);
  for(int i=0; i < MAXPROC; i++){
		pcb_t* pcb= &pcbFree_table[i];
		list_add_tail(&pcb->p_list, &pcbFree_h);
	}
}

void freePcb(pcb_t * p){
	list_add(&p, &pcbFree_h);
}

pcb_t *allocPcb(){
	if(list_empty(&pcbFree_h)) return NULL;
	else {
		pcb_t savedElement = &pcbFree_h->next;
		pcbFree_h->next = pcbFree_h->next->next;
		savedElement.p_child = NULL;
		savedElement.p_list = NULL;
		savedElement.p_parent = NULL;
		savedElement.p_semAdd = NULL;
		savedElement.p_sib = NULL;
		savedElement.p_time = 0;
		return savedElement;
	}
}
//???
void mkEmptyProcQ(struct list_head *head){
	pcb_t list = new pcb_t;
	list.p_list = head;
}

int emptyProcQ(struct list_head *head){
	return list_empty(&head) ? true : false;
}

void insertProcQ(struct list_head *head, pcb_t* p){
	list_add_tail(&p,&head);
}
