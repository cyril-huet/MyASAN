#include "test.h"

static int free_simple(void)
{
    void *p = MY_MALLOC(10);
    CHECK(p != NULL);
    MY_FREE(p);
    CHECK(myasan_get_stats().active_allocations == 0);
    return 1;
}

static int free_null(void)
{
    MY_FREE(NULL);
    CHECK(myasan_get_stats().errors == 0);
    CHECK(myasan_get_stats().total_frees == 0);
    return 1;
}

static int free_stack_pointer(void)
{
    int local = 5;
    MY_FREE(&local);
    CHECK(myasan_last_error() == MYASAN_INVALID_FREE);
    CHECK(myasan_get_stats().errors == 1);
    return 1;
}

static int free_double(void)
{
    void *p = MY_MALLOC(10);
    CHECK(p != NULL);
    MY_FREE(p);
    MY_FREE(p);
    CHECK(myasan_last_error() == MYASAN_DOUBLE_FREE);
    CHECK(myasan_get_stats().total_frees == 1);
    return 1;
}

static int free_two(void)
{
    void *a = MY_MALLOC(1);
    void *b = MY_MALLOC(2);
    CHECK(a != NULL && b != NULL);
    MY_FREE(a);
    MY_FREE(b);
    CHECK(myasan_get_stats().total_frees == 2);
    return 1;
}

static int free_reverse_order(void)
{
    void *a = MY_MALLOC(1);
    void *b = MY_MALLOC(2);
    CHECK(a != NULL && b != NULL);
    MY_FREE(b);
    MY_FREE(a);
    CHECK(myasan_get_stats().active_allocations == 0);
    return 1;
}

static int free_middle(void)
{
    void *a = MY_MALLOC(1);
    void *b = MY_MALLOC(2);
    void *c = MY_MALLOC(3);
    CHECK(a != NULL && b != NULL && c != NULL);
    MY_FREE(b);
    CHECK(myasan_is_tracked(a) == 1);
    CHECK(myasan_is_tracked(c) == 1);
    MY_FREE(a);
    MY_FREE(c);
    return 1;
}

static int finish_one_leak(void)
{
    void *p = MY_MALLOC(5);
    CHECK(p != NULL);
    CHECK(myasan_finish() == 1);
    MY_FREE(p);
    CHECK(myasan_finish() == 0);
    return 1;
}

static int free_removes_bytes(void)
{
    void *a = MY_MALLOC(10);
    void *b = MY_MALLOC(20);
    CHECK(a != NULL && b != NULL);
    MY_FREE(a);
    CHECK(myasan_get_stats().bytes_in_use == 20);
    MY_FREE(b);
    CHECK(myasan_get_stats().bytes_in_use == 0);
    return 1;
}

static int free_double_after_other(void)
{
    void *a = MY_MALLOC(6);
    void *b = MY_MALLOC(9);
    CHECK(a != NULL && b != NULL);
    MY_FREE(a);
    MY_FREE(b);
    MY_FREE(a);
    CHECK(myasan_last_error() == MYASAN_DOUBLE_FREE);
    return 1;
}

static int free_zero_allocation(void)
{
    void *p = MY_MALLOC(0);
    CHECK(p != NULL);
    MY_FREE(p);
    CHECK(myasan_get_stats().total_frees == 1);
    return 1;
}

static int finish_two_leaks(void)
{
    void *a = MY_MALLOC(5);
    void *b = MY_MALLOC(7);
    CHECK(a != NULL && b != NULL);
    CHECK(myasan_finish() == 2);
    MY_FREE(a);
    CHECK(myasan_finish() == 1);
    MY_FREE(b);
    return 1;
}

static int free_marks_not_tracked(void)
{
    void *p = MY_MALLOC(6);
    CHECK(p != NULL);
    MY_FREE(p);
    CHECK(myasan_is_tracked(p) == 0);
    CHECK(myasan_size(p) == 0);
    return 1;
}

static int free_reset_releases(void)
{
    void *p = MY_MALLOC(256);
    CHECK(p != NULL);
    myasan_reset();
    CHECK(myasan_get_stats().active_allocations == 0);
    CHECK(myasan_finish() == 0);
    return 1;
}

static int free_reset_counters(void)
{
    void *p = MY_MALLOC(3);
    CHECK(p != NULL);
    MY_FREE(p);
    myasan_reset();
    CHECK(myasan_get_stats().total_allocations == 0);
    CHECK(myasan_get_stats().total_frees == 0);
    CHECK(myasan_get_stats().errors == 0);
    return 1;
}

void add_free_tests(void)
{
    ADD(free_simple);
    ADD(free_null);
    ADD(free_stack_pointer);
    ADD(free_double);
    ADD(free_two);
    ADD(free_reverse_order);
    ADD(free_middle);
    ADD(finish_one_leak);
    ADD(free_removes_bytes);
    ADD(free_double_after_other);
    ADD(free_zero_allocation);
    ADD(finish_two_leaks);
    ADD(free_marks_not_tracked);
    ADD(free_reset_releases);
    ADD(free_reset_counters);
}
