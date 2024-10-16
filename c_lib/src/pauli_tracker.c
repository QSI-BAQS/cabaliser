#include "pauli_tracker.h"


void* pauli_tracker_create(size_t n_qubits)
{
    return lib_pauli_tracker_create(n_qubits);
}   


void pauli_tracker_destroy(void* tracker)
{
    lib_pauli_tracker_destroy(tracker);
}


void pauli_track_x(
    void* tracker, 
    uintptr_t measured_qubit,
    uintptr_t target_qubit)
{
    lib_pauli_track_x(tracker, measured_qubit, target_qubit);
}

void pauli_track_z(
    void* tracker, 
    uintptr_t measured_qubit,
    uintptr_t target_qubit)
{
    lib_pauli_track_z(tracker, measured_qubit, target_qubit);
}
