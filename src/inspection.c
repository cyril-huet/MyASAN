#include "internal.h"

#include <stdio.h>

size_t myasan_count_leaks(void)
{
    size_t count = 0;
    struct block *block = tracker_first();
    while (block != NULL)
    {
        if (block->freed == 0)
        {
            count++;
        }
        block = block->next;
    }
    return count;
}

size_t myasan_leaked_bytes(void)
{
    size_t bytes = 0;
    struct block *block = tracker_first();
    while (block != NULL)
    {
        if (block->freed == 0)
        {
            bytes += block->size;
        }
        block = block->next;
    }
    return bytes;
}

void myasan_print_summary(void)
{
    struct myasan_stats stats = myasan_get_stats();
    printf("\n--- MyASAN memory summary ---\n");
    printf("Total allocations : %zu\n", stats.total_allocations);
    printf("Total frees       : %zu\n", stats.total_frees);
    printf("Active blocks     : %zu\n", stats.active_allocations);
    printf("Bytes in use      : %zu\n", stats.bytes_in_use);
    printf("Peak bytes        : %zu\n", stats.peak_bytes);
    printf("Errors            : %zu\n", stats.errors);
    printf("Leaked blocks     : %zu\n", myasan_count_leaks());
    printf("Leaked bytes      : %zu\n", myasan_leaked_bytes());
}

void myasan_dump_blocks(void)
{
    struct block *block = tracker_first();
    printf("\n--- MyASAN active allocations ---\n");
    while (block != NULL)
    {
        if (block->freed == 0)
        {
            printf("%p : %zu bytes (%s:%d)\n", (void *)block->data, block->size,
                   block->file, block->line);
        }
        block = block->next;
    }
}
