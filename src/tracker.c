#include "internal.h"

#include <stdlib.h>

static struct block *g_blocks = NULL;

struct block *tracker_add(unsigned char *base, unsigned char *data, size_t size,
                          const char *file, int line)
{
    struct block *block = malloc(sizeof(struct block));
    if (block == NULL)
    {
        return NULL;
    }
    block->base = base;
    block->data = data;
    block->size = size;
    block->file = file;
    block->line = line;
    block->freed = 0;
    block->free_file = NULL;
    block->free_line = 0;
    block->next = g_blocks;
    g_blocks = block;
    return block;
}

struct block *tracker_find(const void *pointer)
{
    struct block *current = g_blocks;
    while (current != NULL)
    {
        if (current->data == pointer)
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

struct block *tracker_first(void) { return g_blocks; }

void tracker_clear(void)
{
    struct block *current = g_blocks;
    while (current != NULL)
    {
        struct block *next = current->next;
        if (current->freed == 0)
        {
            free(current->base);
        }
        free(current);
        current = next;
    }
    g_blocks = NULL;
}
