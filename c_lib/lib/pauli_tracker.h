#ifndef PAULI_TRACKER_H
#define PAULI_TRACKER_H

#include <stddef.h>
#include "lib_pauli_tracker.h" 

struct pauli_frame_t
{
    size_t len;
    void*  elements;
};


struct dependency_t
{
    size_t len;
    size_t* dependencies; 
};

/*
 * pauli_tracker_create
 * Creates a new pauli tracker object
 * :: n_qubits : size_t :: Number of qubits in the tracker 
 * Creates a new opaque object via a rust library binding
 */
void* pauli_tracker_create(size_t n_qubits);
void pauli_tracker_destroy(void* tracker);
void pauli_track_x(
    void* tracker, 
    uintptr_t measured_qubit,
    uintptr_t target_qubit);

void pauli_track_z(
    void* tracker, 
    uintptr_t measured_qubit,
    uintptr_t target_qubit);


#endif
