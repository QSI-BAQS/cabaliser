#include <stdint.h>
#include "pauli_tracker.h"

#ifdef __cplusplus
extern "C" {
#endif

// Single Qubit Operations
void pauli_tracker_ident(MappedPauliTracker *pauli_tracker, uintptr_t arg);
void pauli_tracker_h(MappedPauliTracker *pauli_tracker, uintptr_t arg);
void pauli_tracker_s(MappedPauliTracker *pauli_tracker, uintptr_t arg);
void pauli_tracker_sh(MappedPauliTracker *pauli_tracker, uintptr_t arg);
void pauli_tracker_hs(MappedPauliTracker *pauli_tracker, uintptr_t arg);
void pauli_tracker_shs(MappedPauliTracker *pauli_tracker, uintptr_t arg);


// Two Qubit Operations
void pauli_tracker_cz(MappedPauliTracker *pauli_tracker, uintptr_t ctrl, uintptr_t targ);
void pauli_tracker_cx(MappedPauliTracker *pauli_tracker, uintptr_t ctrl, uintptr_t targ);

#ifdef __cplusplus
}
#endif

