#include "test.h"

static int allocation_one_byte(void)
{
    char *p = MY_MALLOC(1);
    CHECK(p != NULL);
    CHECK(myasan_size(p) == 1);
    MY_FREE(p);
    return 1;
}

static int allocation_zero_bytes(void)
{
    void *p = MY_MALLOC(0);
    CHECK(p != NULL);
    CHECK(myasan_is_tracked(p));
    CHECK(myasan_size(p) == 0);
    MY_FREE(p);
    return 1;
}

static int allocation_one_page(void)
{
    void *p = MY_MALLOC(4096);
    CHECK(p != NULL);
    CHECK(myasan_size(p) == 4096);
    MY_FREE(p);
    return 1;
}

static int allocation_large(void)
{
    char *p = MY_MALLOC(65536);
    CHECK(p != NULL);
    p[65535] = 7;
    CHECK(p[65535] == 7);
    MY_FREE(p);
    return 1;
}

static int allocation_two_distinct(void)
{
    void *a = MY_MALLOC(10);
    void *b = MY_MALLOC(10);
    CHECK(a != NULL && b != NULL);
    CHECK(a != b);
    MY_FREE(a);
    MY_FREE(b);
    return 1;
}

static int allocation_read_write(void)
{
    char *p = MY_MALLOC(20);
    CHECK(p != NULL);
    for (int i = 0; i < 20; i++)
    {
        p[i] = (char)i;
    }
    for (int i = 0; i < 20; i++)
    {
        CHECK(p[i] == i);
    }
    MY_FREE(p);
    return 1;
}

static int allocation_aligned(void)
{
    /* uintptr_t sert uniquement a verifier l'alignement de l'adresse. */
    void *p = MY_MALLOC(sizeof(long double));
    CHECK(p != NULL);
    CHECK((uintptr_t)p % _Alignof(max_align_t) == 0);
    MY_FREE(p);
    return 1;
}

static int allocation_count(void)
{
    void *a = MY_MALLOC(7);
    void *b = MY_MALLOC(8);
    CHECK(a != NULL && b != NULL);
    CHECK(myasan_get_stats().total_allocations == 2);
    MY_FREE(a);
    MY_FREE(b);
    return 1;
}

static int allocation_active_count(void)
{
    void *a = MY_MALLOC(7);
    void *b = MY_MALLOC(8);
    CHECK(a != NULL && b != NULL);
    CHECK(myasan_get_stats().active_allocations == 2);
    MY_FREE(a);
    MY_FREE(b);
    return 1;
}

static int allocation_total_bytes(void)
{
    void *a = MY_MALLOC(7);
    void *b = MY_MALLOC(8);
    CHECK(a != NULL && b != NULL);
    CHECK(myasan_get_stats().bytes_in_use == 15);
    MY_FREE(a);
    MY_FREE(b);
    return 1;
}

static int allocation_peak_bytes(void)
{
    void *a = MY_MALLOC(50);
    CHECK(a != NULL);
    MY_FREE(a);
    void *b = MY_MALLOC(20);
    CHECK(b != NULL);
    CHECK(myasan_get_stats().peak_bytes == 50);
    MY_FREE(b);
    return 1;
}

static int allocation_three_sizes(void)
{
    void *a = MY_MALLOC(3);
    void *b = MY_MALLOC(30);
    void *c = MY_MALLOC(300);
    CHECK(a != NULL && b != NULL && c != NULL);
    CHECK(myasan_size(a) == 3);
    CHECK(myasan_size(b) == 30);
    CHECK(myasan_size(c) == 300);
    MY_FREE(a);
    MY_FREE(b);
    MY_FREE(c);
    return 1;
}

static int allocation_clean_check(void)
{
    void *p = MY_MALLOC(12);
    CHECK(p != NULL);
    CHECK(myasan_check() == 0);
    MY_FREE(p);
    return 1;
}

static int allocation_is_tracked(void)
{
    void *p = MY_MALLOC(12);
    CHECK(p != NULL);
    CHECK(myasan_is_tracked(p) == 1);
    MY_FREE(p);
    return 1;
}

static int allocation_unknown_pointer(void)
{
    int local = 0;
    CHECK(myasan_is_tracked(&local) == 0);
    CHECK(myasan_size(&local) == 0);
    return 1;
}

void add_allocation_tests(void)
{
    ADD(allocation_one_byte);
    ADD(allocation_zero_bytes);
    ADD(allocation_one_page);
    ADD(allocation_large);
    ADD(allocation_two_distinct);
    ADD(allocation_read_write);
    ADD(allocation_aligned);
    ADD(allocation_count);
    ADD(allocation_active_count);
    ADD(allocation_total_bytes);
    ADD(allocation_peak_bytes);
    ADD(allocation_three_sizes);
    ADD(allocation_clean_check);
    ADD(allocation_is_tracked);
    ADD(allocation_unknown_pointer);
}
