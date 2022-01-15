#include "pcb.h"

//array di PCB con dimensione massima di MAXPROC
pcb_t pcbFree_table[MAXPROC];
PTRPcbFree_h pcbFree_h;

LIST_HEAD(pcbFree_h);

void initPcbs(void){
	for(int i=0; i < MAXPROC; i++){
		pcb_t* pcb= &pcbFree_table[i];
		list_add_tail(&(pcb->p_list.next),&(pcbFree_h));
	}
}