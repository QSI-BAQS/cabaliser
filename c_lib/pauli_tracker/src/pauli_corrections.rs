use pauli_tracker::{
    pauli::{PauliDense},
};

//use std::mem::ManuallyDrop;

use crate::const_vec::{
    ConstVec,
    vec_to_const_vec, 
//    const_vec_destroy, 
};

use crate::{
    mapped_pauli_tracker,
};

type PauliVec = Vec<Vec<PauliDense>>;


#[no_mangle]
extern "C" fn lib_pauli_tracker_create_pauli_corrections(
    tracker: *const mapped_pauli_tracker::MappedPauliTracker
    ) -> *const PauliVec
{
    unsafe {
        let pauli_tracker = &(tracker.as_ref().unwrap().pauli_tracker);

//        let dense_rep = ManuallyDrop::new(pauli_tracker.transpose::<PauliDense>(
//                    pauli_tracker.as_storage().len()
//            ));

        return Box::into_raw(
            Box::new(
                pauli_tracker.transpose::<PauliDense>(
                    pauli_tracker.as_storage().len()
                ) 
            )
        );
    }
}

#[no_mangle]
extern "C" fn lib_pauli_tracker_get_correction_table_len(
    corrections: *mut Vec<Vec<PauliDense>>
) -> usize
{
    unsafe {
        return corrections.as_ref().unwrap().len(); 
    }
}

#[no_mangle]
extern "C" fn lib_pauli_tracker_get_pauli_corrections(
    corrections: *mut Vec<Vec<PauliDense>>,
    index: usize
) -> *mut ConstVec<PauliDense>
{
    unsafe {
        return vec_to_const_vec::<PauliDense>(
        &(corrections.as_ref().unwrap()[index])
        );
    }
}

//#[no_mangle]
//extern "C" fn lib_pauli_tracker_destroy_corrections(
//    vec: *mut ConstVec<PauliDense>
//)
//{
//    const_vec_destroy::<PauliDense>(vec);
//}
