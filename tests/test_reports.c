#include "test.h"

#define LOG_PATH "tests/myasan_test.log"

/* On lit le fichier avec une simple boucle, sans outil de test externe. */
static int log_contains(const char *word)
{
    char text[4096];
    size_t count = 0;
    FILE *file = fopen(LOG_PATH, "r");
    if (file == NULL)
    {
        return 0;
    }
    int letter = fgetc(file);
    while (letter != EOF && count < sizeof(text) - 1)
    {
        text[count] = (char)letter;
        count++;
        letter = fgetc(file);
    }
    text[count] = '\0';
    fclose(file);
    return strstr(text, word) != NULL;
}

static int start_log(void)
{
    myasan_close_log();
    remove(LOG_PATH);
    return myasan_set_log_file(LOG_PATH) == 0;
}

static int report_log_opens(void)
{
    CHECK(start_log());
    myasan_close_log();
    FILE *file = fopen(LOG_PATH, "r");
    CHECK(file != NULL);
    fclose(file);
    return 1;
}

static int report_reject_null_path(void)
{
    CHECK(myasan_set_log_file(NULL) == -1);
    return 1;
}

static int report_reject_invalid_path(void)
{
    CHECK(myasan_set_log_file("tests/missing_dir/not_here.log") == -1);
    return 1;
}

static int report_double_close(void)
{
    CHECK(start_log());
    myasan_close_log();
    myasan_close_log();
    return 1;
}

static int report_invalid_free_text(void)
{
    CHECK(start_log());
    int n = 4;
    MY_FREE(&n);
    CHECK(log_contains("invalid free"));
    return 1;
}

static int report_double_free_text(void)
{
    CHECK(start_log());
    void *p = MY_MALLOC(8);
    CHECK(p != NULL);
    MY_FREE(p);
    MY_FREE(p);
    CHECK(log_contains("double free"));
    return 1;
}

static int report_allocation_location(void)
{
    CHECK(start_log());
    void *p = MY_MALLOC(8);
    CHECK(p != NULL);
    MY_FREE(p);
    MY_FREE(p);
    CHECK(log_contains("Allocated at: tests/test_reports.c:"));
    return 1;
}

static int report_first_free_location(void)
{
    CHECK(start_log());
    void *p = MY_MALLOC(8);
    CHECK(p != NULL);
    MY_FREE(p);
    MY_FREE(p);
    CHECK(log_contains("First freed at: tests/test_reports.c:"));
    return 1;
}

static int report_leak_text(void)
{
    CHECK(start_log());
    void *p = MY_MALLOC(23);
    CHECK(p != NULL);
    CHECK(myasan_finish() == 1);
    CHECK(log_contains("LEAK: 23 bytes"));
    MY_FREE(p);
    return 1;
}

static int report_size_overflow_text(void)
{
    CHECK(start_log());
    CHECK(MY_MALLOC(SIZE_MAX) == NULL);
    CHECK(log_contains("size overflow"));
    return 1;
}

static int report_underflow_text(void)
{
    CHECK(start_log());
    unsigned char *p = MY_MALLOC(3);
    CHECK(p != NULL);
    p[-1] = 0;
    CHECK(myasan_check() == 1);
    CHECK(log_contains("buffer underflow"));
    MY_FREE(p);
    return 1;
}

static int report_overflow_text(void)
{
    CHECK(start_log());
    unsigned char *p = MY_MALLOC(3);
    CHECK(p != NULL);
    p[3] = 0;
    CHECK(myasan_check() == 1);
    CHECK(log_contains("buffer overflow"));
    MY_FREE(p);
    return 1;
}

static int report_silent_console_still_logs(void)
{
    CHECK(start_log());
    myasan_set_reporting(0);
    MY_FREE((void *)"invalid");
    CHECK(log_contains("invalid free"));
    return 1;
}

static int report_normal_mode_no_alloc_text(void)
{
    CHECK(start_log());
    myasan_set_verbose(0);
    void *p = MY_MALLOC(11);
    CHECK(p != NULL);
    MY_FREE(p);
    CHECK(!log_contains("[MyASAN] ALLOC:"));
    return 1;
}

static int report_verbose_allocation(void)
{
    CHECK(start_log());
    myasan_set_verbose(1);
    void *p = MY_MALLOC(12);
    CHECK(p != NULL);
    CHECK(log_contains("ALLOC: 12 bytes"));
    MY_FREE(p);
    return 1;
}

static int report_verbose_free(void)
{
    CHECK(start_log());
    myasan_set_verbose(1);
    void *p = MY_MALLOC(13);
    CHECK(p != NULL);
    MY_FREE(p);
    CHECK(log_contains("FREE: 13 bytes"));
    return 1;
}

static int report_disable_verbose(void)
{
    CHECK(start_log());
    myasan_set_verbose(1);
    myasan_set_verbose(0);
    void *p = MY_MALLOC(15);
    CHECK(p != NULL);
    MY_FREE(p);
    CHECK(!log_contains("ALLOC:"));
    return 1;
}

static int report_log_reopen_truncates(void)
{
    CHECK(start_log());
    int n = 3;
    MY_FREE(&n);
    CHECK(log_contains("invalid free"));
    CHECK(myasan_set_log_file(LOG_PATH) == 0);
    CHECK(!log_contains("invalid free"));
    return 1;
}

static int report_log_failure_preserves_old(void)
{
    CHECK(start_log());
    CHECK(myasan_set_log_file("tests/missing_dir/oops.log") == -1);
    int n = 4;
    MY_FREE(&n);
    CHECK(log_contains("invalid free"));
    return 1;
}

static int report_stop_logging(void)
{
    CHECK(start_log());
    myasan_close_log();
    int n = 5;
    MY_FREE(&n);
    CHECK(!log_contains("invalid free"));
    return 1;
}

void add_report_tests(void)
{
    ADD(report_log_opens);
    ADD(report_reject_null_path);
    ADD(report_reject_invalid_path);
    ADD(report_double_close);
    ADD(report_invalid_free_text);
    ADD(report_double_free_text);
    ADD(report_allocation_location);
    ADD(report_first_free_location);
    ADD(report_leak_text);
    ADD(report_size_overflow_text);
    ADD(report_underflow_text);
    ADD(report_overflow_text);
    ADD(report_silent_console_still_logs);
    ADD(report_normal_mode_no_alloc_text);
    ADD(report_verbose_allocation);
    ADD(report_verbose_free);
    ADD(report_disable_verbose);
    ADD(report_log_reopen_truncates);
    ADD(report_log_failure_preserves_old);
    ADD(report_stop_logging);
}
