#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdlib.h>
#include <string.h>

#define NODE_TO_OBJ(node) (node + sizeof(list_node_t))

struct linked_list_t
{
    size_t n_elements;
    struct list_node_t* head;
    struct list_node_t* tail;
};

struct list_node_t
{
    struct list_node_t* next; 
    size_t node_size;
};


struct linked_list_t* linked_list_create();
void linked_list_destroy(struct linked_list_t* ll);
void linked_list_add(struct linked_list_t* ll, void* obj, size_t n_bytes);



#endif
