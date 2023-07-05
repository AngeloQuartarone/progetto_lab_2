#include "queue.h"
#include "util.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

static int en = 0;

/**
 * Initialize the queue
 * 
 * @param q queue to initialize
 */
void init(Queue *q)
{
	q->head = q->tail = NULL;
	en = pthread_mutex_init(&q->mutex, NULL);
	handle_en_error(en, "pthread_mutex_init in init");
	en = pthread_cond_init(&q->cond, NULL);
	handle_en_error(en, "pthread_cond_init in init");
}

/**
 * Destroy the queue
 * 
 * @param q queue to destroy
 */
void destroy(Queue *q)
{
	en = pthread_mutex_destroy(&q->mutex);
	handle_en_error(en, "pthread_mutex_destroy in destroy");
	en = pthread_cond_destroy(&q->cond);
	handle_en_error(en, "pthread_cond_destroy in destroy");
	while (q->head)
	{
		Node *temp __attribute__((unused))= q->head;
		q->head = q->head->next;
	}
	q->tail = NULL;
}

/**
 * Push an element in the queue
 * 
 * @param data data to push
 * @param q queue where to push the data
 */
void push(void *data, Queue *q)
{
	Node *n = (Node *)malloc(sizeof(Node));
	if (n == NULL)
	{
		dprintf(2, "malloc: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	n->data = data;
	n->next = NULL;

	en = pthread_mutex_lock(&q->mutex);
	handle_en_error(en, "pthread_mutex_lock in push");
	if (q->tail == NULL)
	{
		q->head = n;
	}
	else
	{
		q->tail->next = n;
	}
	q->tail = n;

	en = pthread_cond_signal(&q->cond);
	handle_en_error(en, "pthread_cond_signal in push");
	en = pthread_mutex_unlock(&q->mutex);
	handle_en_error(en, "pthread_mutex_unlock in push");
}

/**
 * Pop an element from the queue
 * 
 * @param q queue where to pop the data
 * @return data popped
 */
void *pop(Queue *q)
{
    en = pthread_mutex_lock(&q->mutex);
    handle_en_error(en, "pthread_mutex_lock in pop");

    while (q->head == NULL)
    {
        en = pthread_cond_wait(&q->cond, &q->mutex);
		handle_en_error(en, "pthread_cond_wait in pop");
    }

    Node *n = q->head;
    q->head = q->head->next;
    if (q->head == NULL)
        q->tail = NULL;
    en = pthread_mutex_unlock(&q->mutex);
    handle_en_error(en, "pthread_mutex_unlock in pop");

    void *data = n->data;
    free(n);
    return data;
}

/**
 * Pop an element from the queue without blocking
 * 
 * @param q queue where to pop the data
 * @return data popped
 */
void *non_blocking_pop(Queue *q)
{
    pthread_mutex_lock(&q->mutex);

    if (q->head == NULL) {
        en = pthread_mutex_unlock(&q->mutex);
		handle_en_error(en, "pthread_mutex_unlock in non_blocking_pop");
        return NULL;
    }

    Node *n = q->head;
    q->head = q->head->next;
    if (q->head == NULL)
        q->tail = NULL;

    en = pthread_mutex_unlock(&q->mutex);
	handle_en_error(en, "pthread_mutex_unlock in non_blocking_pop");

    void *data = n->data;
    free(n);
    return data;
}
