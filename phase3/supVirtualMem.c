#include "./headers/VMSupport.h"

swap_t swap_table[POOLSIZE];
int swapSem;

void initSwapTable(){
    swapSem = 1;
    for (int i = 0; i < POOLSIZE; i++)
        swap_table[i].sw_asid = NOPROC;
}


