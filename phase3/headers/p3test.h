#ifndef P3TEST_H_INCLUDED
#define P3TEST_H_INCLUDED

#include "../../generic_headers/pandos_const.h"
#include "../../generic_headers/pandos_types.h"
#include "../../phase1/headers/listx.h"
#include <umps/arch.h>
#include <umps/libumps.h>
#include "supSyscall.h"
#include "supVM.h"
#include "TLBhandler.h"

#define myprint(s) \
    klog_print(s); \
    bp()

static int master_sem; // master sem to controll the end of the uproc

static support_t sd_table[UPROCMAX]; // table of usable support descriptor
static struct list_head sd_free;     // list of free support descriptor

static swap_t swap_pool_table[POOLSIZE];
static int swap_pool_sem;

void test();
void init_sds();
void init_sd_free();
support_t *alloc_sd();
void free_sd(support_t *s);
void create_uproc(int asid);
void run_proc();

#endif