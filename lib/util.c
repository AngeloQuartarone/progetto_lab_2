#include "util.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>

#define BUF_SIZE 512

/**
 * Handles a null pointer error.
 * @param ptr pointer to check for null.
 * @param msg message to print if ptr is null.
 */
void handle_null_error(void *ptr, char *msg)
{
    if (ptr == NULL)
    {
        perror(msg);
        exit(EXIT_FAILURE);
    }
}

/**
 * Handles an error with an error number.
 * @param en error number.
 * @param msg message to print if en is not 0.
 */
void handle_en_error(int *en, char *msg)
{
    if (en != 0)
    {
        errno = en;
        perror(msg);
        exit(EXIT_FAILURE);
    }
}

/**
 * Do a safe free of a pointer. If the pointer is not null, it is freed and set to null.
 * 
 * @param ptr pointer to free. 
 */
void safe_free(void *ptr)
{
    if (ptr != NULL)
    {
        free(ptr);
        ptr = NULL;
    }
}

/**
 * Prints a message to stderr.
 */
void message(pthread_mutex_t *mess_log, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    pthread_mutex_t *m = mess_log;
    pthread_mutex_lock(m);
    dprintf(2, fmt, args);
    pthread_mutex_unlock(m);
    va_end(args);
    return;
}

/**
 * Return the size of a pointer. 
 * 
 * @param ptr pointer to check.
 * @return size_t size of the pointer.
 */
size_t size_of_ptr(char *ptr){
    size_t size = 0;
    while(*ptr != '\0'){
        size++;
        ptr++;
    }
    return size;
}