#include "test.h"

static int calloc_one(void)
{
    int *p = MY_CALLOC(1, sizeof(int));
    CHECK(p != NULL);
    CHECK(*p == 0);
    MY_FREE(p);
    return 1;
}

static int calloc_zero_count(void)
{
    void *p = MY_CALLOC(0, 10);
    CHECK(p != NULL);
    CHECK(myasan_size(p) == 0);
    MY_FREE(p);
    return 1;
}

static int calloc_zero_size(void)
{
    void *p = MY_CALLOC(10, 0);
    CHECK(p != NULL);
    CHECK(myasan_size(p) == 0);
    MY_FREE(p);
    return 1;
}

static int calloc_all_zero(void)
{
    unsigned char *p = MY_CALLOC(100, 1);
    CHECK(p != NULL);
    for (int i = 0; i < 100; i++)
    {
        CHECK(p[i] == 0);
    }
    MY_FREE(p);
    return 1;
}

static int calloc_size(void)
{
    int *p = MY_CALLOC(6, sizeof(int));
    CHECK(p != NULL);
    CHECK(myasan_size(p) == 6 * sizeof(int));
    MY_FREE(p);
    return 1;
}

static int calloc_overflow(void)
{
    void *p = MY_CALLOC(SIZE_MAX, 2);
    CHECK(p == NULL);
    CHECK(myasan_last_error() == MYASAN_SIZE_OVERFLOW);
    return 1;
}

static int calloc_too_big(void)
{
    void *p = MY_CALLOC(1, SIZE_MAX);
    CHECK(p == NULL);
    CHECK(myasan_last_error() == MYASAN_SIZE_OVERFLOW);
    return 1;
}

static int calloc_free(void)
{
    void *p = MY_CALLOC(10, 4);
    CHECK(p != NULL);
    MY_FREE(p);
    CHECK(myasan_get_stats().bytes_in_use == 0);
    return 1;
}

static int realloc_null(void)
{
    void *p = MY_REALLOC(NULL, 15);
    CHECK(p != NULL);
    CHECK(myasan_size(p) == 15);
    MY_FREE(p);
    return 1;
}

static int realloc_grows(void)
{
    char *p = MY_MALLOC(2);
    CHECK(p != NULL);
    p[0] = 'A';
    p[1] = 'B';
    char *bigger = MY_REALLOC(p, 20);
    CHECK(bigger != NULL);
    CHECK(bigger[0] == 'A' && bigger[1] == 'B');
    CHECK(myasan_size(bigger) == 20);
    MY_FREE(bigger);
    return 1;
}

static int realloc_shrinks(void)
{
    char *p = MY_MALLOC(10);
    CHECK(p != NULL);
    p[0] = 'X';
    char *smaller = MY_REALLOC(p, 1);
    CHECK(smaller != NULL);
    CHECK(smaller[0] == 'X');
    MY_FREE(smaller);
    return 1;
}

static int realloc_same_size(void)
{
    char *p = MY_MALLOC(10);
    CHECK(p != NULL);
    p[0] = 42;
    char *next = MY_REALLOC(p, 10);
    CHECK(next != NULL);
    CHECK(next[0] == 42);
    MY_FREE(next);
    return 1;
}

static int realloc_zero(void)
{
    void *p = MY_MALLOC(5);
    CHECK(p != NULL);
    void *next = MY_REALLOC(p, 0);
    CHECK(next == NULL);
    CHECK(myasan_get_stats().active_allocations == 0);
    return 1;
}

static int realloc_invalid(void)
{
    int number = 5;
    void *p = MY_REALLOC(&number, 10);
    CHECK(p == NULL);
    CHECK(myasan_last_error() == MYASAN_INVALID_REALLOC);
    return 1;
}

static int calloc_then_realloc(void)
{
    int *p = MY_CALLOC(2, sizeof(int));
    CHECK(p != NULL);
    CHECK(p[0] == 0);
    int *next = MY_REALLOC(p, 10 * sizeof(int));
    CHECK(next != NULL);
    CHECK(next[0] == 0);
    MY_FREE(next);
    return 1;
}

static int realloc_twice(void)
{
    char *p = MY_MALLOC(2);
    CHECK(p != NULL);
    char *q = MY_REALLOC(p, 4);
    CHECK(q != NULL);
    char *r = MY_REALLOC(q, 8);
    CHECK(r != NULL);
    CHECK(myasan_size(r) == 8);
    MY_FREE(r);
    return 1;
}

static int realloc_larger_block(void)
{
    char *p = MY_MALLOC(128);
    CHECK(p != NULL);
    p[127] = 'Z';
    char *q = MY_REALLOC(p, 8192);
    CHECK(q != NULL);
    CHECK(q[127] == 'Z');
    MY_FREE(q);
    return 1;
}

static int realloc_preserves_content(void)
{
    unsigned char *p = MY_MALLOC(50);
    CHECK(p != NULL);
    for (int i = 0; i < 50; i++)
    {
        p[i] = (unsigned char)i;
    }
    unsigned char *q = MY_REALLOC(p, 60);
    CHECK(q != NULL);
    for (int i = 0; i < 50; i++)
    {
        CHECK(q[i] == i);
    }
    MY_FREE(q);
    return 1;
}

static int realloc_statistics(void)
{
    void *p = MY_MALLOC(10);
    CHECK(p != NULL);
    void *q = MY_REALLOC(p, 20);
    CHECK(q != NULL);
    CHECK(myasan_get_stats().active_allocations == 1);
    CHECK(myasan_get_stats().bytes_in_use == 20);
    CHECK(myasan_get_stats().total_allocations == 2);
    MY_FREE(q);
    return 1;
}

static int realloc_freed_block(void)
{
    void *p = MY_MALLOC(10);
    CHECK(p != NULL);
    MY_FREE(p);
    void *q = MY_REALLOC(p, 20);
    CHECK(q == NULL);
    CHECK(myasan_last_error() == MYASAN_INVALID_REALLOC);
    return 1;
}

void add_calloc_realloc_tests(void)
{
    ADD(calloc_one);
    ADD(calloc_zero_count);
    ADD(calloc_zero_size);
    ADD(calloc_all_zero);
    ADD(calloc_size);
    ADD(calloc_overflow);
    ADD(calloc_too_big);
    ADD(calloc_free);
    ADD(realloc_null);
    ADD(realloc_grows);
    ADD(realloc_shrinks);
    ADD(realloc_same_size);
    ADD(realloc_zero);
    ADD(realloc_invalid);
    ADD(calloc_then_realloc);
    ADD(realloc_twice);
    ADD(realloc_larger_block);
    ADD(realloc_preserves_content);
    ADD(realloc_statistics);
    ADD(realloc_freed_block);
}
