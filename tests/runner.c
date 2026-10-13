#include "test.h"

struct test_case
{
    const char *name;
    int (*run)(void);
};

static struct test_case g_tests[100];
static size_t g_count = 0;

void register_test(const char *name, int (*test)(void))
{
    if (g_count >= 100)
    {
        fprintf(stderr, "Trop de tests enregistres\n");
        exit(1);
    }
    g_tests[g_count].name = name;
    g_tests[g_count].run = test;
    g_count++;
}

static int run_one_test(size_t index)
{
    myasan_reset();
    myasan_close_log();
    myasan_set_verbose(0);
    myasan_set_reporting(0);
    int result = g_tests[index].run();
    const char *status = "FAIL";
    if (result != 0)
    {
        status = "OK";
    }
    printf("[%s] %02zu/100 %s\n", status, index + 1, g_tests[index].name);
    myasan_reset();
    myasan_close_log();
    myasan_set_verbose(0);
    remove("tests/myasan_test.log");
    return result;
}

int main(void)
{
    add_allocation_tests();
    add_free_tests();
    add_calloc_realloc_tests();
    add_guard_tests();
    add_inspection_tests();
    add_report_tests();
    if (g_count != 100)
    {
        fprintf(stderr, "Attendu : 100 tests, recu : %zu\n", g_count);
        return 1;
    }
    size_t passed = 0;
    for (size_t i = 0; i < g_count; i++)
    {
        if (run_one_test(i))
        {
            passed++;
        }
    }
    printf("\nResultat : %zu/100 tests reussis\n", passed);
    if (passed != 100)
    {
        return 1;
    }
    return 0;
}
