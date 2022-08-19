#ifndef P3TEST_H_INCLUDED
#define P3TEST_H_INCLUDED

#include "../../generic_headers/pandos_const.h"
#include "../../generic_headers/pandos_types.h"

void test()
void init_ds();
void init_sd_free();
support_t alloc_sd();
void free_sd(support_t *s);
void run_test();

#endif