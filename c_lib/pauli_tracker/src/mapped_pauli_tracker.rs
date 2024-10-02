use pauli_tracker::{
    collection::{Init, Map},
    pauli::{self},
    tracker::{Tracker, frames::Frames},
};

// Intermediate type wrappers 
type BitVec = bitvec::vec::BitVec;
type PauliStack = pauli::PauliStack<BitVec>;

// Define the specific types that you need
type PauliTracker = Frames::<Map<PauliStack>>;

/*
 * Pauli Tracker with mapping 
 */
#[derive(Debug)]
pub struct MappedPauliTracker {
    pub pauli_tracker : PauliTracker,  // Pauli tracker object 
    pub mapper : Vec<usize>, // Array where indicies represent rows and values represent measured qubits
}

/*
 * pauli_tracker_create
 * Constructor for the pauli tracker object
 * :: n_qubits : usize :: Number of initialised qubits in the tracker
 * Returns a pointer to a dynamically allocated MappedPauliTracker object  
 */
#[no_mangle]
pub extern "C" fn pauli_tracker_create(n_qubits: usize) -> *mut MappedPauliTracker {
    return Box::into_raw(
        Box::new(
            MappedPauliTracker {
                pauli_tracker : PauliTracker::init(n_qubits),
                mapper : Vec::new(),
            }
        )
    );
}

/*
 * pauli_tracker_destroy
 * Destructor for the pauli tracker object
 * :: pauli_tracker : *mut MappedPauliTracker :: Pointer to the pauli tracker object to be freed 
 * Acts in place to free the pauli tracker object
 */
#[no_mangle]
extern "C" fn pauli_tracker_destroy(pauli_tracker: *mut MappedPauliTracker) {
    unsafe {
        let _ = Box::from_raw(pauli_tracker);
    }
}

/*
 * pauli_track_x
 * Add a row to the pauli tracker object with an 'X' at the target qubit  
 * All other locations will be the identity
 * :: pauli_tracker : &mut MappedPauliTracker :: Pauli tracker object  
 * :: measured_qubit : usize :: The qubit being measured 
 * :: measurement_target : usize :: The qubit with the conditional pauli 
 * Acts in place on the Pauli tracker object
 */
#[no_mangle]
extern "C" fn pauli_track_x(mapped_pauli_tracker: &mut MappedPauliTracker, measured_qubit: usize, measurement_target: usize) {
    mapped_pauli_tracker.mapper.push(measured_qubit);
    mapped_pauli_tracker.pauli_tracker.track_x(measurement_target);
}

/*
 * pauli_tracker_greedy_order
 * Returns the measurement order and dependencies 
 * :: mapped_pauli_tracker : &MappedPauliTracker :: Pauli tracker object to print 
 * Acts in place, and writes to stdout
 */
#[no_mangle]
extern "C" fn pauli_tracker_greedy_order(mapped_pauli_tracker: &MappedPauliTracker) {
    

}

/*
 * pauli_tracker_print
 * Small printing function for the pauli tracker
 * :: mapped_pauli_tracker : &MappedPauliTracker :: Pauli tracker object to print 
 * Acts in place, and writes to stdout
 */
#[no_mangle]
extern "C" fn pauli_tracker_print(mapped_pauli_tracker: &MappedPauliTracker) {
    println!("{mapped_pauli_tracker:?}");
}
