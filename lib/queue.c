#include "queue.h"
#include "util.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

static int en;
static void *ptr;

void init(Queue *q)
{
	q->head = q->tail = NULL;
	en = pthread_mutex_init(&q->mutex, NULL);
	handle_en_error(en, "pthread_mutex_init in init");
	en = pthread_cond_init(&q->cond, NULL);
	handle_en_error(en, "pthread_cond_init in init");
}

void destroy(Queue *q)
{
	pthread_mutex_destroy(&q->mutex);
	pthread_cond_destroy(&q->cond);
	while (q->head)
	{
		Node *temp = q->head;
		q->head = q->head->next;
	}
	q->tail = NULL;
}

void push(void *data, Queue *q)
{
	Node *n = (Node *)malloc(sizeof(Node));
	n->data = data;
	n->next = NULL;

	int s = pthread_mutex_lock(&q->mutex);
	if (s != 0)
	{
		dprintf(2, "pthread_mutex_lock: %s\n", strerror(s));
		exit(1);
	}
	if (q->tail == NULL)
	{
		q->head = n;
	}
	else
	{
		q->tail->next = n;
	}
	q->tail = n;

	s = pthread_cond_signal(&q->cond);
	if (s != 0)
	{
		dprintf(2, "pthread_cond_signal: %s\n", strerror(s));
		exit(1);
	}
	s = pthread_mutex_unlock(&q->mutex);
	if (s != 0)
	{
		dprintf(2, "pthread_mutex_unlock: %s\n", strerror(s));
		exit(1);
	}
}

void *pop(Queue *q)
{
    int s = pthread_mutex_lock(&q->mutex);
    if (s != 0)
    {
        dprintf(2, "pthread_mutex_lock: %s\n", strerror(s));
        exit(1);
    }

    while (q->head == NULL)
    {
        s = pthread_cond_wait(&q->cond, &q->mutex);
        if (s != 0)
        {
            dprintf(2, "pthread_cond_wait: %s\n", strerror(s));
            exit(1);
        }
    }

    Node *n = q->head;
    q->head = q->head->next;
    if (q->head == NULL)
        q->tail = NULL;
    s = pthread_mutex_unlock(&q->mutex);
    if (s != 0)
    {
        dprintf(2, "pthread_mutex_unlock: %s\n", strerror(s));
        exit(1);
    }

    void *data = n->data;
    free(n);
    return data;
}

void *non_blocking_pop(Queue *q)
{
    pthread_mutex_lock(&q->mutex);

    if (q->head == NULL) {
        pthread_mutex_unlock(&q->mutex);
        return NULL;
    }

    Node *n = q->head;
    q->head = q->head->next;
    if (q->head == NULL)
        q->tail = NULL;

    pthread_mutex_unlock(&q->mutex);

    void *data = n->data;
    free(n);
    return data;
}
