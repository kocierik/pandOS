#include "phase1/headers/asl.h"
#include "phase1/headers/listx.h"
#include "phase1/headers/pandos_const.h"
#include "phase1/headers/pandos_types.h"
#include "phase1/headers/pcb.h"

ssize_t active_proc;
ssize_t blocked_proc;
static LIST_HEAD(ready_proc);
pcb_t *curr_active_proc;

struct semDev{
  unsigned short value; // boolean value | [0] off | [1] on |
  semd_t sem;   
  semDev *next;
};

void initGlobalVar(){
  active_proc = 0;
  blocked_proc = 0;
  curr_active_proc = NULL;
}
 

int main(int argc, int* argv[]){
  //Inizializzazione fase 1
  initPcbs();
  initSemd();

  //Inizializzare le variabili dichiarate precedentemente
  initGlobalVar();

  //Popolare il pass up vector con gestore e stack pointer per eccezioni e TLB-Refill
  
  // ...

  return 0;
}