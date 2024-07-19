#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdlib.h>

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


#endif
