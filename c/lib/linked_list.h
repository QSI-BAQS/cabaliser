#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdlib.h>
#include <string.h>

#define NODE_TO_OBJ(node) (node + sizeof(list_node_t))

struct linked_list_t
{
    size_t n_elements;
    size_t n_reuse_elements;
    struct list_node_t* head;
    struct list_node_t* tail;
    struct list_node_t* reuse_head; // Forms a stack of re-usable nodes, this cuts down on reallocations
};

struct list_node_t
{
    struct list_node_t* prev; 
    void* obj;
};


struct linked_list_t* linked_list_create();
void linked_list_destroy(struct linked_list_t* ll);
void linked_list_push(struct linked_list_t* ll, void* obj);
void* linked_list_pop(struct linked_list_t* ll);




#endif
