#include "test.h"

static int inspect_empty(void)
{
    CHECK(myasan_count_leaks() == 0);
    CHECK(myasan_leaked_bytes() == 0);
    return 1;
}

static int inspect_one_leak(void)
{
    char *p = MY_MALLOC(12);
    CHECK(p != NULL);
    CHECK(myasan_count_leaks() == 1);
    CHECK(myasan_leaked_bytes() == 12);
    return 1;
}

static int inspect_two_leaks(void)
{
    char *a = MY_MALLOC(3);
    char *b = MY_MALLOC(7);
    CHECK(a != NULL && b != NULL);
    CHECK(myasan_count_leaks() == 2);
    CHECK(myasan_leaked_bytes() == 10);
    return 1;
}

static int inspect_after_free(void)
{
    char *p = MY_MALLOC(19);
    CHECK(p != NULL);
    MY_FREE(p);
    CHECK(myasan_count_leaks() == 0);
    CHECK(myasan_leaked_bytes() == 0);
    return 1;
}

static int inspect_mixed_blocks(void)
{
    char *a = MY_MALLOC(5);
    char *b = MY_MALLOC(15);
    CHECK(a != NULL && b != NULL);
    MY_FREE(a);
    CHECK(myasan_count_leaks() == 1);
    CHECK(myasan_leaked_bytes() == 15);
    MY_FREE(b);
    return 1;
}

static int inspect_zero_size(void)
{
    char *p = MY_MALLOC(0);
    CHECK(p != NULL);
    CHECK(myasan_count_leaks() == 1);
    CHECK(myasan_leaked_bytes() == 0);
    MY_FREE(p);
    return 1;
}

static int inspect_after_realloc_grow(void)
{
    char *p = MY_MALLOC(4);
    CHECK(p != NULL);
    char *bigger = MY_REALLOC(p, 25);
    CHECK(bigger != NULL);
    CHECK(myasan_count_leaks() == 1);
    CHECK(myasan_leaked_bytes() == 25);
    MY_FREE(bigger);
    return 1;
}

static int inspect_after_realloc_shrink(void)
{
    char *p = MY_MALLOC(24);
    CHECK(p != NULL);
    char *smaller = MY_REALLOC(p, 2);
    CHECK(smaller != NULL);
    CHECK(myasan_count_leaks() == 1);
    CHECK(myasan_leaked_bytes() == 2);
    MY_FREE(smaller);
    return 1;
}

static int inspect_calloc_bytes(void)
{
    char *p = MY_CALLOC(3, 6);
    CHECK(p != NULL);
    CHECK(myasan_count_leaks() == 1);
    CHECK(myasan_leaked_bytes() == 18);
    MY_FREE(p);
    return 1;
}

static int inspect_repeat_is_read_only(void)
{
    char *p = MY_MALLOC(8);
    CHECK(p != NULL);
    CHECK(myasan_count_leaks() == 1);
    CHECK(myasan_count_leaks() == 1);
    CHECK(myasan_leaked_bytes() == 8);
    CHECK(myasan_get_stats().errors == 0);
    MY_FREE(p);
    return 1;
}

static int inspect_valid_pointer(void)
{
    char *p = MY_MALLOC(3);
    CHECK(p != NULL);
    CHECK(myasan_check_pointer(p) == 0);
    CHECK(myasan_last_error() == MYASAN_OK);
    MY_FREE(p);
    return 1;
}

static int inspect_underflow(void)
{
    unsigned char *p = MY_MALLOC(4);
    CHECK(p != NULL);
    p[-1] = 0;
    CHECK(myasan_check_pointer(p) == 1);
    CHECK(myasan_last_error() == MYASAN_UNDERFLOW);
    MY_FREE(p);
    return 1;
}

static int inspect_overflow(void)
{
    unsigned char *p = MY_MALLOC(4);
    CHECK(p != NULL);
    p[4] = 0;
    CHECK(myasan_check_pointer(p) == 1);
    CHECK(myasan_last_error() == MYASAN_OVERFLOW);
    MY_FREE(p);
    return 1;
}

static int inspect_both_guards(void)
{
    unsigned char *p = MY_MALLOC(4);
    CHECK(p != NULL);
    p[-1] = 0;
    p[4] = 0;
    CHECK(myasan_check_pointer(p) == 2);
    CHECK(myasan_last_error() == MYASAN_OVERFLOW);
    MY_FREE(p);
    return 1;
}

static int inspect_freed_pointer(void)
{
    char *p = MY_MALLOC(9);
    CHECK(p != NULL);
    MY_FREE(p);
    CHECK(myasan_check_pointer(p) == -1);
    CHECK(myasan_last_error() == MYASAN_INVALID_CHECK);
    return 1;
}

static int inspect_unknown_pointer(void)
{
    int value = 4;
    CHECK(myasan_check_pointer(&value) == -1);
    CHECK(myasan_last_error() == MYASAN_INVALID_CHECK);
    CHECK(myasan_get_stats().errors == 1);
    return 1;
}

static int inspect_null_pointer(void)
{
    CHECK(myasan_check_pointer(NULL) == -1);
    CHECK(myasan_last_error() == MYASAN_INVALID_CHECK);
    return 1;
}

static int inspect_valid_after_error(void)
{
    char *p = MY_MALLOC(3);
    CHECK(p != NULL);
    CHECK(myasan_check_pointer(NULL) == -1);
    CHECK(myasan_check_pointer(p) == 0);
    CHECK(myasan_last_error() == MYASAN_OK);
    CHECK(myasan_get_stats().errors == 1);
    MY_FREE(p);
    return 1;
}

static int inspect_one_of_two_pointers(void)
{
    unsigned char *a = MY_MALLOC(4);
    unsigned char *b = MY_MALLOC(4);
    CHECK(a != NULL && b != NULL);
    a[4] = 0;
    CHECK(myasan_check_pointer(b) == 0);
    CHECK(myasan_get_stats().errors == 0);
    MY_FREE(a);
    MY_FREE(b);
    return 1;
}

static int inspect_after_reset(void)
{
    char *p = MY_MALLOC(3);
    CHECK(p != NULL);
    myasan_reset();
    CHECK(myasan_count_leaks() == 0);
    CHECK(myasan_leaked_bytes() == 0);
    CHECK(myasan_get_stats().errors == 0);
    return 1;
}

void add_inspection_tests(void)
{
    ADD(inspect_empty);
    ADD(inspect_one_leak);
    ADD(inspect_two_leaks);
    ADD(inspect_after_free);
    ADD(inspect_mixed_blocks);
    ADD(inspect_zero_size);
    ADD(inspect_after_realloc_grow);
    ADD(inspect_after_realloc_shrink);
    ADD(inspect_calloc_bytes);
    ADD(inspect_repeat_is_read_only);
    ADD(inspect_valid_pointer);
    ADD(inspect_underflow);
    ADD(inspect_overflow);
    ADD(inspect_both_guards);
    ADD(inspect_freed_pointer);
    ADD(inspect_unknown_pointer);
    ADD(inspect_null_pointer);
    ADD(inspect_valid_after_error);
    ADD(inspect_one_of_two_pointers);
    ADD(inspect_after_reset);
}
