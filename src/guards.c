#include "internal.h"

#include <stddef.h>

#define GUARD_BYTE 171

/* sizeof(max_align_t) garde le pointeur rendu correctement aligne. */
size_t guards_length(void) { return sizeof(max_align_t); }

void guards_write(struct block *block)
{
    size_t length = guards_length();
    for (size_t i = 0; i < length; i++)
    {
        block->base[i] = GUARD_BYTE;
        block->data[block->size + i] = GUARD_BYTE;
    }
}

int guards_before_ok(const struct block *block)
{
    for (size_t i = 0; i < guards_length(); i++)
    {
        if (block->base[i] != GUARD_BYTE)
        {
            return 0;
        }
    }
    return 1;
}

int guards_after_ok(const struct block *block)
{
    for (size_t i = 0; i < guards_length(); i++)
    {
        if (block->data[block->size + i] != GUARD_BYTE)
        {
            return 0;
        }
    }
    return 1;
}
