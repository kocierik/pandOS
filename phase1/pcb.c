#include "pcb.h"
#include "listx.h"

//array di PCB con dimensione massima di MAXPROC
pcb_t pcbFree_table[MAXPROC];
struct list_head pcbFree_h;

void initPcbs(void){
  pcbFree_h = LIST_HEAD_INIT(&pcbFree_h);
  for(int i=0; i < MAXPROC; i++){
		pcb_t* pcb= &pcbFree_table[i];
		list_add_tail(&(pcb->p_list),&(pcbFree_h));
	}
}