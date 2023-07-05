#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>



void handle_null_error(void *, char *);
void handle_en_error(int , char *);
void safe_free(void *);
void message(pthread_mutex_t *, char *, ...);
size_t size_of_ptr(char *ptr);

#endif