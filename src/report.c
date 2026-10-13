#include "internal.h"

#include <stdio.h>

static int g_reporting = 1;
static int g_verbose = 0;
static FILE *g_log = NULL;

static const char *error_name(enum myasan_error error)
{
    if (error == MYASAN_INVALID_FREE)
    {
        return "invalid free";
    }
    if (error == MYASAN_DOUBLE_FREE)
    {
        return "double free";
    }
    if (error == MYASAN_INVALID_REALLOC)
    {
        return "invalid realloc";
    }
    if (error == MYASAN_INVALID_CHECK)
    {
        return "invalid pointer check";
    }
    if (error == MYASAN_UNDERFLOW)
    {
        return "buffer underflow";
    }
    if (error == MYASAN_OVERFLOW)
    {
        return "buffer overflow";
    }
    if (error == MYASAN_SIZE_OVERFLOW)
    {
        return "size overflow";
    }
    return "unknown error";
}

void report_set_enabled(int enabled) { g_reporting = enabled != 0; }

void report_set_verbose(int enabled) { g_verbose = enabled != 0; }

int report_set_log_file(const char *path)
{
    if (path == NULL)
    {
        return -1;
    }
    FILE *file = fopen(path, "w");
    if (file == NULL)
    {
        return -1;
    }
    report_close_log();
    g_log = file;
    return 0;
}

void report_close_log(void)
{
    if (g_log != NULL)
    {
        fclose(g_log);
        g_log = NULL;
    }
}

static void write_error(FILE *output, enum myasan_error error, const char *file,
                        int line, const struct block *block)
{
    fprintf(output, "[MyASAN] ERROR: %s at %s:%d\n", error_name(error), file,
            line);
    if (block != NULL)
    {
        fprintf(output, "  Pointer: %p, size: %zu\n", (void *)block->data,
                block->size);
        fprintf(output, "  Allocated at: %s:%d\n", block->file, block->line);
        if (block->free_file != NULL)
        {
            fprintf(output, "  First freed at: %s:%d\n", block->free_file,
                    block->free_line);
        }
    }
}

void report_error(enum myasan_error error, const char *file, int line,
                  const struct block *block)
{
    if (g_reporting != 0)
    {
        write_error(stderr, error, file, line, block);
    }
    if (g_log != NULL)
    {
        write_error(g_log, error, file, line, block);
        fflush(g_log);
    }
}

static void write_leak(FILE *output, const struct block *block)
{
    fprintf(output, "[MyASAN] LEAK: %zu bytes, pointer %p, at %s:%d\n",
            block->size, (void *)block->data, block->file, block->line);
}

void report_leak(const struct block *block)
{
    if (g_reporting != 0)
    {
        write_leak(stderr, block);
    }
    if (g_log != NULL)
    {
        write_leak(g_log, block);
        fflush(g_log);
    }
}

static void write_event(FILE *output, const char *action,
                        const struct block *block)
{
    fprintf(output, "[MyASAN] %s: %zu bytes, pointer %p, at %s:%d\n", action,
            block->size, (void *)block->data, block->file, block->line);
}

void report_event(const char *action, const struct block *block)
{
    if (g_verbose == 0)
    {
        return;
    }
    if (g_reporting != 0)
    {
        write_event(stderr, action, block);
    }
    if (g_log != NULL)
    {
        write_event(g_log, action, block);
        fflush(g_log);
    }
}
