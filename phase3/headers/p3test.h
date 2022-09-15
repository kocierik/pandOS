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
#include "../../phase2/headers/globals.h"

#define myprint(s) \
    klog_print(s); \
    bp()

void test();
void init_sds();
void init_sd_free();
support_t *alloc_sd();
void free_sd(support_t *s);
void create_uproc(int asid);
void run_proc();

#endif