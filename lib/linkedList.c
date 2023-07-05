#include "linkedList.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

static int en = 0;

/**
 * Initializes the linked list.
 * @param list pointer to the linked list to initialize.
 */
void initialize_list(linked_list_t *list)
{
    list->head = NULL;
    en = pthread_mutex_init(&list->lock, NULL);
    handle_en_error(en, "pthread_mutex_init in initialize_list");
    list->size = 0;
}

/**
 * Adds a node to the linked list.
 * Node is added to the head of the linked list and
 * the size of the linked list is incremented.
 * @param list pointer to the linked list to add to.
 * @param data pointer to the data to add to the linked list.
 */
void add_node(linked_list_t *list, void *data)
{
    node_t *new_node = (node_t *)calloc(1, sizeof(node_t));
    handle_null_error(new_node, "calloc");
    new_node->data = data;
    en = pthread_mutex_lock(&list->lock);
    handle_en_error(en, "pthread_mutex_lock in add_node");
    new_node->next = list->head;
    list->head = new_node;
    list->size = list->size + 1;
    en = pthread_mutex_unlock(&list->lock);
    handle_en_error(en, "pthread_mutex_unlock in add_node");
}

/**
 * Removes a node from the linked list.
 * Node is removed from the linked list,
 * the size of the linked list is decremented.
 * @param list pointer to the linked list to remove from.
 * @param data pointer to the data to remove from the linked list.
 */
void remove_node(linked_list_t *list, void *data)
{
    en = pthread_mutex_lock(&list->lock);
    handle_en_error(en, "pthread_mutex_lock in remove_node");
    node_t *current = list->head;
    node_t *previous = NULL;
    while (current != NULL)
    {
        if (current->data == data)
        {
            if (previous == NULL)
            {
                list->head = current->next;
            }
            else
            {
                previous->next = current->next;
            }

            safe_free(current);
            list->size = list->size - 1;
            break;
        }
        else
        {
            previous = current;
            current = current->next;
        }
    }

    if (current == NULL)
    {
        //message("linked_list.c", "remove_node", "data not found in linked list");
    }

    en = pthread_mutex_unlock(&list->lock);
    handle_en_error(en, "pthread_mutex_unlock in remove_node");
}

/**
 * Gets the nth element from the linked list.
 * @param list pointer to the linked list to get the nth element from.
 * @param n the index of the element to get.
 * @return pointer to the nth element.
 */
void *get_nth_element(linked_list_t *list, int n)
{
    if (n < 0 || n >= list->size)
    {
        return NULL;
    }
    en = pthread_mutex_lock(&list->lock);
    handle_en_error(en, "pthread_mutex_lock in get_nth_element");
    node_t *current = list->head;
    for (int i = 0; i < n; i++)
    {
        current = current->next;
    }
    void *data = current->data;
    en = pthread_mutex_unlock(&list->lock);
    handle_en_error(en, "pthread_mutex_unlock in get_nth_element");
    return data;
}

/**
 * Frees the linked list.
 * @param list pointer to the linked list to free.
 */
void free_list(linked_list_t *list)
{
    en = pthread_mutex_lock(&list->lock);
    node_t *current = list->head;
    while (current != NULL)
    {
        node_t *next = current->next;
        safe_free(current->data);
        safe_free(current);
        current = next;
    }
    list->head = NULL;
    list->size = 0;
    pthread_mutex_destroy(&list->lock);
}