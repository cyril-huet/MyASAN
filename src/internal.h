#ifndef MYASAN_INTERNAL_H
#define MYASAN_INTERNAL_H

#include "myasan.h"

/* Un bloc garde deux petites zones protegees autour des donnees. */
struct block
{
    unsigned char *base;
    unsigned char *data;
    size_t size;
    const char *file;
    int line;
    int freed;
    const char *free_file;
    int free_line;
    struct block *next;
};

struct block *tracker_add(unsigned char *base, unsigned char *data, size_t size,
                          const char *file, int line);
struct block *tracker_find(const void *pointer);
struct block *tracker_first(void);
void tracker_clear(void);

size_t guards_length(void);
void guards_write(struct block *block);
int guards_before_ok(const struct block *block);
int guards_after_ok(const struct block *block);

void report_error(enum myasan_error error, const char *file, int line,
                  const struct block *block);
void report_event(const char *action, const struct block *block);
void report_leak(const struct block *block);
void report_set_enabled(int enabled);
void report_set_verbose(int enabled);
int report_set_log_file(const char *path);
void report_close_log(void);

#endif
