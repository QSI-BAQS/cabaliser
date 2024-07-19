#ifndef ARRAY_LIST_H
#ifndef ARRAY_LIST_H


struct array_list_t
{
    size_t n_elements; 
    size_t max_elements;
    void* elements;
    size_t element_size; 
}; 
typedef struct array_list_t array_list_t;

// Constructor
array_list_t* array_list_create(const size_t n_elements, const size_t element_size);

array_list_t* add_element(array_list_t* arr, void* element);

// Destructor
void array_list_destroy(array_list_t* arr);

#endif
