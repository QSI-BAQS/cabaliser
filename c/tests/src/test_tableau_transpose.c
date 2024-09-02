#include <string.h>

#include "tableau.h"

void print(uint64_t** arr_a, const size_t n_channels, const size_t x_offset, const size_t y_offset_bytes, const size_t y_offset_bits)
{
    for (size_t i = 0; i < n_channels; i++)
    {
        printf("|");
        for (size_t j = 0; j < n_channels; j++)
        {
            printf("%d ", !!((arr_a[j + x_offset][y_offset_bytes] >> y_offset_bits) & (1 << i)));
        }
        printf("|\n");
    } 
printf("\n");
}


tableau_t* tableau_random_create(size_t n_qubits)
{
    tableau_t* tab = tableau_create(n_qubits);
    for (size_t i = 0; i < n_qubits; i++)
    {
        for (size_t j = 0; j < tab->slice_len; j++)
        {
           tab->slices_x[i][j] = rand(); 
           tab->slices_z[i][j] = rand(); 
        }
    }     
    tab->phases[0] = rand(); 

    return tab;
} 

tableau_t* tableau_copy(tableau_t* tab)
{
    tableau_t* tab_cpy = tableau_random_create(tab->n_qubits); 
    #pragma omp parallel for
    for (size_t i = 0; i < tab->n_qubits; i++)
    {
        for (size_t j = 0; j < tab->slice_len; j++)
        {
            tab_cpy->slices_x[i][j] = tab->slices_x[i][j]; 
            tab_cpy->slices_z[i][j] = tab->slices_z[i][j]; 
        }
    }   
    for (size_t j = 0; j < tab->slice_len; j++)
    {
        tab_cpy->phases[j] = tab->phases[j]; 
    }
    return tab_cpy;
}


void test_small(const size_t n_qubits)
{
    tableau_t* tab = tableau_random_create(n_qubits); 
    tableau_t* tab_cmp = tableau_copy(tab);  

    print(tab->slices_x, 16, 16, 0, 0);
    //print(tab_cmp->slices_x, 16, 16, 0, 0);

    tableau_transpose(tab);  
    tableau_transpose_naive(tab_cmp);

    print(tab->slices_x, 16, 0, 0, 16);
    //print(tab_cmp->slices_x, 16, 0, 0, 16);


    for (size_t i = 0; i < tab->n_qubits; i++)
    {
        for (size_t j = 0; j < tab->slice_len; j++)
        {
            //printf("%d %d\n",(uint8_t)(tab->slices_x[i][0]), (uint8_t)(tab_cmp->slices_x[i][0]));
            assert(((uint8_t)(tab->slices_x[i][0])) == ((uint8_t)(tab_cmp->slices_x[i][0])));
            //assert(tab->slices_z[i][j] == tab_cmp->slices_z[i][j]);
        }
    } 
    return;
}


int main()
{
    srand(3);
    test_small(64);

//    for (size_t i = 3; i < 256; i++)
//    {
//        test_small(i);
//    }

    return 0;
}
