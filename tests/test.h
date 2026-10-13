#ifndef MYASAN_TEST_H
#define MYASAN_TEST_H

#include "myasan.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition)                                                       \
    do                                                                         \
    {                                                                          \
        if (!(condition))                                                      \
        {                                                                      \
            fprintf(stderr, "ECHEC %s:%d : %s\n", __FILE__, __LINE__,          \
                    #condition);                                               \
            return 0;                                                          \
        }                                                                      \
    } while (0)

#define ADD(test) register_test(#test, test)

void register_test(const char *name, int (*test)(void));
void add_allocation_tests(void);
void add_free_tests(void);
void add_calloc_realloc_tests(void);
void add_guard_tests(void);
void add_inspection_tests(void);
void add_report_tests(void);

#endif
