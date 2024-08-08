#include "linked_list.h"

struct linked_list_t* linked_list_create()
{
    struct linked_list_t* ll = (struct linked_list_t*)malloc(sizeof(struct linked_list_t));
    ll->n_elements = 0;
    ll->head = NULL;
    ll->tail = NULL;
    ll->reuse_head = NULL;
    return ll;
}


static inline
void __add_reusable_node(struct linked_list_t* ll, struct list_node_t* node)
{
    if (NULL == ll->reuse_head)
    {
        ll->reuse_head = node; 
        node->prev = NULL;
    }
    else
    {
        node->prev = ll->reuse_head;
        ll->reuse_head = node;
    }
}


static inline
void* __pop_reusable_node(struct linked_list_t* ll)
{
    if (NULL == ll->reuse_head)
    {
        return malloc(sizeof(struct list_node_t*));
    }
    else
    {
        struct list_node_t* node = ll->reuse_head;
        ll->reuse_head = node->prev;
        return node;
    }
}


struct linked_list_t* linked_list_create_prealloc(const size_t n_prealloced)
{
    struct linked_list_t* ll = linked_list_create();

    struct list_node_t* nodes = (struct list_node_t*)malloc(sizeof(struct list_node_t) * n_prealloced); 

    for (size_t i = 0; i < n_prealloced; i++)
    {
        __add_reusable_node(ll, nodes + i);
    }

    return ll;
}


void linked_list_destroy(struct linked_list_t* ll)
{
    struct list_node_t* next = ll->tail;
    for (struct list_node_t* iter = ll->tail; next != NULL; iter = next)
    {
        next = iter->prev; 
        free(iter->obj);
        free(iter);
    } 
    for (struct list_node_t* iter = ll->reuse_head; next != NULL; iter = next)
    {
        next = iter->prev; 
        free(iter);
    } 

    free(ll);
}

/*
 * linked_list_push
 * Adds an element to the linked list
 * :: ll : struct linked_list* :: Linked list to add element to
 * :: obj : void* :: Pointer to object to copy to list
 * Acts in place on the linked list, appends to the tail
 */
void linked_list_push(struct linked_list_t* ll, void* obj) 
{
    struct list_node_t* node = __pop_reusable_node(ll); 
 
    if (NULL == ll->head)
    {
        ll->head = node;
        ll->tail = node;
    }
    else
    {
        ll->head->prev = node;
    }
    node->prev = NULL;
    ll->head = node;
 
    node->obj = obj;
    return;
}


void* linked_list_pop(struct linked_list_t* ll)
{
    if (ll->n_elements == 0)
    {
        return NULL;
    }
    struct list_node_t* node = ll->tail;

    void* obj = node->obj; 
    node->obj = NULL;

    ll->tail = node->prev; 

    __add_reusable_node(ll, node);
    return obj;
}
