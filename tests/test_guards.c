#include "test.h"

static int guard_detect_overflow(void)
{
    unsigned char *p = MY_MALLOC(10);
    CHECK(p != NULL);
    p[10] = 1;
    CHECK(myasan_check() == 1);
    CHECK(myasan_last_error() == MYASAN_OVERFLOW);
    MY_FREE(p);
    return 1;
}

static int guard_detect_underflow(void)
{
    unsigned char *p = MY_MALLOC(10);
    CHECK(p != NULL);
    p[-1] = 1;
    CHECK(myasan_check() == 1);
    CHECK(myasan_last_error() == MYASAN_UNDERFLOW);
    MY_FREE(p);
    return 1;
}

static int guard_detect_both(void)
{
    unsigned char *p = MY_MALLOC(10);
    CHECK(p != NULL);
    p[-1] = 1;
    p[10] = 1;
    CHECK(myasan_check() == 2);
    CHECK(myasan_get_stats().errors == 2);
    MY_FREE(p);
    return 1;
}

static int guard_clean_memory(void)
{
    unsigned char *p = MY_MALLOC(10);
    CHECK(p != NULL);
    p[0] = 1;
    p[9] = 1;
    CHECK(myasan_check() == 0);
    MY_FREE(p);
    return 1;
}

static int guard_check_no_change(void)
{
    char *p = MY_MALLOC(5);
    CHECK(p != NULL);
    p[0] = 'H';
    CHECK(myasan_check() == 0);
    CHECK(p[0] == 'H');
    MY_FREE(p);
    return 1;
}

static int guard_detect_when_free(void)
{
    unsigned char *p = MY_MALLOC(3);
    CHECK(p != NULL);
    p[3] = 0;
    MY_FREE(p);
    CHECK(myasan_last_error() == MYASAN_OVERFLOW);
    CHECK(myasan_get_stats().errors == 1);
    return 1;
}

static int guard_after_realloc(void)
{
    unsigned char *p = MY_MALLOC(3);
    CHECK(p != NULL);
    unsigned char *q = MY_REALLOC(p, 8);
    CHECK(q != NULL);
    q[8] = 0;
    CHECK(myasan_check() == 1);
    MY_FREE(q);
    return 1;
}

static int guard_after_calloc(void)
{
    unsigned char *p = MY_CALLOC(3, 4);
    CHECK(p != NULL);
    p[12] = 0;
    CHECK(myasan_check() == 1);
    MY_FREE(p);
    return 1;
}

static int guard_only_corrupted_block(void)
{
    unsigned char *a = MY_MALLOC(3);
    unsigned char *b = MY_MALLOC(8);
    CHECK(a != NULL && b != NULL);
    a[3] = 0;
    CHECK(myasan_check() == 1);
    MY_FREE(a);
    MY_FREE(b);
    return 1;
}

static int guard_empty_check_and_finish(void)
{
    CHECK(myasan_check() == 0);
    CHECK(myasan_finish() == 0);
    CHECK(myasan_last_error() == MYASAN_OK);
    return 1;
}

void add_guard_tests(void)
{
    ADD(guard_detect_overflow);
    ADD(guard_detect_underflow);
    ADD(guard_detect_both);
    ADD(guard_clean_memory);
    ADD(guard_check_no_change);
    ADD(guard_detect_when_free);
    ADD(guard_after_realloc);
    ADD(guard_after_calloc);
    ADD(guard_only_corrupted_block);
    ADD(guard_empty_check_and_finish);
}
