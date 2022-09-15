#ifndef VAR_H_INCLUDED
#define VAR_H_INCLUDED

#include "../../generic_headers/pandos_types.h"
#include "../../phase1/headers/listx.h"


extern int master_sem; // master sem to controll the end of the uproc

extern support_t sd_table[UPROCMAX]; // table of usable support descriptor
extern struct list_head sd_free;     // list of free support descriptor

extern swap_t swap_pool_table[POOLSIZE];
extern int swap_pool_sem;

#endif