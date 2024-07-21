#include "linked_list.h"

struct linked_list_t* linked_list_create()
{
    struct linked_list_t* ll = malloc(sizeof(struct linked_list_t));
    ll->n_elements = 0;
    ll->head = NULL;
    ll->tail = NULL;
    return ll;
}

void linked_list_destroy(struct linked_list_t* ll)
{
    struct list_node_t* next = ll->head;
    for (struct list_node_t* iter = ll->head; next != NULL; iter = next)
    {
        next = iter->next; 
        free(iter);
    } 
    free(ll);
}

/*
 * linked_list_add
 * Adds an element to the linked list
 * :: ll : struct linked_list* :: Linked list to add element to
 * :: obj : void* :: Pointer to object to copy to list
 * :: n_bytes : size_t :: Number of bytes to copy
 * Acts in place on the linked list, appends to the tail
 */
void linked_list_add(struct linked_list_t* ll, void* obj, size_t n_bytes) 
{
    struct list_node_t* node = malloc(sizeof(struct list_node_t) + n_bytes); 
 
    if (NULL == ll->head)
    {
        ll->head = node;
    }
 
    if (NULL != ll->tail)
    {    
        ll->tail->next = node;
    } 
    
    node->next = NULL;
    ll->tail = node;
 
    // Copy the object over
    memcpy(((void*)node) + sizeof(struct list_node_t), obj, n_bytes); 
    return;
}
