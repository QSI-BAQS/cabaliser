#include <stdint.h>

typedef struct MappedPauliTracker MappedPauliTracker;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * pauli_tracker_create
 * Constructor for the pauli tracker object
 * :: n_qubits : usize :: Number of initialised qubits in the tracker
 * Returns a pointer to a dynamically allocated MappedPauliTracker object  
 */
MappedPauliTracker *pauli_tracker_create(uintptr_t n_qubits);


/*
 * pauli_tracker_destroy
 * Destructor for the pauli tracker object
 * :: pauli_tracker : *mut MappedPauliTracker :: Pointer to the pauli tracker object to be freed 
 * Acts in place to free the pauli tracker object
 */
void pauli_tracker_destroy(MappedPauliTracker *pauli_tracker);

/*
 * pauli_track_x
 * Add a row to the pauli tracker object with an 'X' at the target qubit  
 * All other locations will be the identity
 * :: pauli_tracker : &mut MappedPauliTracker :: Pauli tracker object  
 * :: measured_qubit : usize :: The qubit being measured 
 * :: measurement_target : usize :: The qubit with the conditional pauli 
 * Acts in place on the Pauli tracker object
 */
void pauli_track_x(MappedPauliTracker *pauli_tracker, uintptr_t measured_qubit, uintptr_t measurement_target);

/*
 * pauli_tracker_print
 * Small printing function for the pauli tracker
 * :: mapped_pauli_tracker : &MappedPauliTracker :: Pauli tracker object to print 
 * Acts in place, and writes to stdout
 */
void pauli_tracker_print(const MappedPauliTracker* const mapped_pauli_tracker);



#ifdef __cplusplus
}
#endif
