#include "internal.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static struct myasan_stats g_stats;
static enum myasan_error g_last_error = MYASAN_OK;

static void save_error(enum myasan_error error, const char *file, int line,
                       const struct block *block)
{
    g_last_error = error;
    g_stats.errors++;
    report_error(error, file, line, block);
}

static int has_room(size_t size)
{
    size_t guard = guards_length();
    if (guard > SIZE_MAX / 2)
    {
        return 0;
    }
    return size <= SIZE_MAX - guard * 2;
}

static void count_allocation(size_t size)
{
    g_stats.active_allocations++;
    g_stats.total_allocations++;
    g_stats.bytes_in_use += size;
    if (g_stats.bytes_in_use > g_stats.peak_bytes)
    {
        g_stats.peak_bytes = g_stats.bytes_in_use;
    }
}

void *myasan_malloc(size_t size, const char *file, int line)
{
    g_last_error = MYASAN_OK;
    if (!has_room(size))
    {
        save_error(MYASAN_SIZE_OVERFLOW, file, line, NULL);
        return NULL;
    }
    size_t length = guards_length();
    unsigned char *base = malloc(size + length * 2);
    if (base == NULL)
    {
        return NULL;
    }
    unsigned char *data = base + length;
    struct block *block = tracker_add(base, data, size, file, line);
    if (block == NULL)
    {
        free(base);
        return NULL;
    }
    guards_write(block);
    count_allocation(size);
    report_event("ALLOC", block);
    return data;
}

void *myasan_calloc(size_t count, size_t size, const char *file, int line)
{
    g_last_error = MYASAN_OK;
    if (count != 0 && size > SIZE_MAX / count)
    {
        save_error(MYASAN_SIZE_OVERFLOW, file, line, NULL);
        return NULL;
    }
    size_t total = count * size;
    void *pointer = myasan_malloc(total, file, line);
    if (pointer != NULL)
    {
        memset(pointer, 0, total);
    }
    return pointer;
}

static void verify_block(const struct block *block, const char *file, int line)
{
    if (!guards_before_ok(block))
    {
        save_error(MYASAN_UNDERFLOW, file, line, block);
    }
    if (!guards_after_ok(block))
    {
        save_error(MYASAN_OVERFLOW, file, line, block);
    }
}

void myasan_free(void *pointer, const char *file, int line)
{
    g_last_error = MYASAN_OK;
    if (pointer == NULL)
    {
        return;
    }
    struct block *block = tracker_find(pointer);
    if (block == NULL)
    {
        save_error(MYASAN_INVALID_FREE, file, line, NULL);
        return;
    }
    if (block->freed != 0)
    {
        save_error(MYASAN_DOUBLE_FREE, file, line, block);
        return;
    }
    verify_block(block, file, line);
    block->free_file = file;
    block->free_line = line;
    report_event("FREE", block);
    free(block->base);
    block->base = NULL;
    block->freed = 1;
    g_stats.active_allocations--;
    g_stats.total_frees++;
    g_stats.bytes_in_use -= block->size;
}

void *myasan_realloc(void *pointer, size_t size, const char *file, int line)
{
    g_last_error = MYASAN_OK;
    if (pointer == NULL)
    {
        return myasan_malloc(size, file, line);
    }
    struct block *block = tracker_find(pointer);
    if (block == NULL || block->freed != 0)
    {
        save_error(MYASAN_INVALID_REALLOC, file, line, block);
        return NULL;
    }
    if (size == 0)
    {
        myasan_free(pointer, file, line);
        return NULL;
    }
    size_t copy_size = block->size;
    if (copy_size > size)
    {
        copy_size = size;
    }
    void *new_pointer = myasan_malloc(size, file, line);
    if (new_pointer == NULL)
    {
        return NULL;
    }
    memcpy(new_pointer, pointer, copy_size);
    myasan_free(pointer, file, line);
    return new_pointer;
}

int myasan_check(void)
{
    size_t before = g_stats.errors;
    struct block *block = tracker_first();
    g_last_error = MYASAN_OK;
    while (block != NULL)
    {
        if (block->freed == 0)
        {
            verify_block(block, block->file, block->line);
        }
        block = block->next;
    }
    return (int)(g_stats.errors - before);
}

size_t myasan_finish(void)
{
    size_t leaks = 0;
    struct block *block = tracker_first();
    myasan_check();
    while (block != NULL)
    {
        if (block->freed == 0)
        {
            report_leak(block);
            leaks++;
        }
        block = block->next;
    }
    return leaks;
}

void myasan_reset(void)
{
    tracker_clear();
    memset(&g_stats, 0, sizeof(g_stats));
    g_last_error = MYASAN_OK;
}

void myasan_set_reporting(int enabled) { report_set_enabled(enabled); }

void myasan_set_verbose(int enabled) { report_set_verbose(enabled); }

int myasan_set_log_file(const char *path) { return report_set_log_file(path); }

void myasan_close_log(void) { report_close_log(); }

struct myasan_stats myasan_get_stats(void) { return g_stats; }

enum myasan_error myasan_last_error(void) { return g_last_error; }

int myasan_is_tracked(const void *pointer)
{
    struct block *block = tracker_find(pointer);
    if (block == NULL)
    {
        return 0;
    }
    return block->freed == 0;
}

size_t myasan_size(const void *pointer)
{
    struct block *block = tracker_find(pointer);
    if (block == NULL || block->freed != 0)
    {
        return 0;
    }
    return block->size;
}

/* -1 : pointeur inconnu ou libere, sinon nombre de gardes abimees. */
int myasan_check_pointer(const void *pointer)
{
    g_last_error = MYASAN_OK;
    struct block *block = tracker_find(pointer);
    if (block == NULL || block->freed != 0)
    {
        save_error(MYASAN_INVALID_CHECK, __FILE__, __LINE__, block);
        return -1;
    }
    size_t before = g_stats.errors;
    verify_block(block, block->file, block->line);
    return (int)(g_stats.errors - before);
}
