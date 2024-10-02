use super::mapped_pauli_tracker::{
    MappedPauliTracker,
};

use pauli_tracker::tracker::Tracker;

// Single Qubit Gates
#[no_mangle]
extern "C" fn pauli_tracker_ident(_: &mut MappedPauliTracker, _: usize){}

#[no_mangle]
extern "C" fn pauli_tracker_h(
    mapped_pauli_tracker: &mut MappedPauliTracker,
    arg: usize)
{
    mapped_pauli_tracker.pauli_tracker.h(arg);   
}

#[no_mangle]
extern "C" fn pauli_tracker_s(
    mapped_pauli_tracker: &mut MappedPauliTracker,
    arg: usize)
{
    mapped_pauli_tracker.pauli_tracker.s(arg);   
}

#[no_mangle]
extern "C" fn pauli_tracker_sh(
    mapped_pauli_tracker: &mut MappedPauliTracker,
    arg: usize)
{
    mapped_pauli_tracker.pauli_tracker.sh(arg);   
}

#[no_mangle]
extern "C" fn pauli_tracker_hs(
    mapped_pauli_tracker: &mut MappedPauliTracker,
    arg: usize)
{
    mapped_pauli_tracker.pauli_tracker.hs(arg);   
}

#[no_mangle]
extern "C" fn pauli_tracker_shs(
    mapped_pauli_tracker: &mut MappedPauliTracker,
    arg: usize)
{
    mapped_pauli_tracker.pauli_tracker.shs(arg);   
}

// Two Qubit Gates
#[no_mangle]
extern "C" fn pauli_tracker_cx(
    mapped_pauli_tracker: &mut MappedPauliTracker,
    source: usize,
    target: usize)
{
    mapped_pauli_tracker.pauli_tracker.cx(source, target);   
}

#[no_mangle]
extern "C" fn pauli_tracker_cz(
    mapped_pauli_tracker: &mut MappedPauliTracker,
    source: usize,
    target: usize)
{
    mapped_pauli_tracker.pauli_tracker.cz(source, target);   
}
