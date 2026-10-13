#ifndef MYASAN_H
#define MYASAN_H

#include <stddef.h>

/* Les erreurs que MyASAN sait reconnaitre. */
enum myasan_error
{
    MYASAN_OK = 0,
    MYASAN_INVALID_FREE,
    MYASAN_DOUBLE_FREE,
    MYASAN_INVALID_REALLOC,
    MYASAN_INVALID_CHECK,
    MYASAN_UNDERFLOW,
    MYASAN_OVERFLOW,
    MYASAN_SIZE_OVERFLOW
};

struct myasan_stats
{
    size_t active_allocations;
    size_t total_allocations;
    size_t total_frees;
    size_t bytes_in_use;
    size_t peak_bytes;
    size_t errors;
};

void *myasan_malloc(size_t size, const char *file, int line);
void *myasan_calloc(size_t count, size_t size, const char *file, int line);
void *myasan_realloc(void *pointer, size_t size, const char *file, int line);
void myasan_free(void *pointer, const char *file, int line);

int myasan_check(void);
size_t myasan_finish(void);
void myasan_reset(void);
void myasan_set_reporting(int enabled);
void myasan_set_verbose(int enabled);
int myasan_set_log_file(const char *path);
void myasan_close_log(void);
struct myasan_stats myasan_get_stats(void);
enum myasan_error myasan_last_error(void);
int myasan_is_tracked(const void *pointer);
size_t myasan_size(const void *pointer);

/* Fonctions pour inspecter la memoire facilement. */
int myasan_check_pointer(const void *pointer);
size_t myasan_count_leaks(void);
size_t myasan_leaked_bytes(void);
void myasan_print_summary(void);
void myasan_dump_blocks(void);

/* Pour utiliser l'API sans remplacer les noms standards. */
#define MY_MALLOC(size) myasan_malloc((size), __FILE__, __LINE__)
#define MY_CALLOC(count, size)                                                 \
    myasan_calloc((count), (size), __FILE__, __LINE__)
#define MY_REALLOC(pointer, size)                                              \
    myasan_realloc((pointer), (size), __FILE__, __LINE__)
#define MY_FREE(pointer) myasan_free((pointer), __FILE__, __LINE__)

/* Mode pratique : a definir APRES les en-tetes standards du programme. */
#ifdef MYASAN_REPLACE
#define malloc(size) MY_MALLOC(size)
#define calloc(count, size) MY_CALLOC(count, size)
#define realloc(pointer, size) MY_REALLOC(pointer, size)
#define free(pointer) MY_FREE(pointer)
#endif

#endif
