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
		list_del(savedElement);
		LIST_HEAD_INIT(&pcbFree_h);
		return savedElement;
	}
}