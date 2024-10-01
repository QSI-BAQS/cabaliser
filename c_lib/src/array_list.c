#include "array_list.h"

struct array_list_t* array_list_create(const size_t max_elements, const size_t element_size)
{
    struct array_list_t* al = (struct array_list_t*)malloc(sizeof(struct array_list_t)); 

    al->n_elements = 0;
    al->max_elements = max_elements;
    al->element_size = element_size;
    al->elements = malloc(element_size * max_elements);   
    return al;
}


void array_list_append(struct array_list_t* al, void* item)
{
    // Array full, double and copy
    if (al->n_elements == al->max_elements - 1)
    {
        al->max_elements *= 2;    
        al->elements = reallocarray(al->elements, al->max_elements, al->element_size);
    }
    memcpy(item, ((char*)(al->elements)) + (al->n_elements * al->element_size), al->element_size); 
    al->n_elements++;
       
    return;
} 
