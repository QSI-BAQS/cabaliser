use pauli_tracker::{
    tracker::{frames::induced_order::{self}},
    collection::Iterable,
};
pub use pauli_tracker::tracker::{frames::induced_order::{PartialOrderGraph}};

use super::mapped_pauli_tracker::{
    MappedPauliTracker,
};

/*
 * C array-like wrapper
 */
#[repr(C)]
pub struct ConstVec<T> {
    len : u32,
    ptr : *const T,
    step : u32, 
}

type Dependents = Vec<usize>;

#[repr(C)]
pub struct DependentNode {
    index : u32,
    dependencies : Dependents,
}

pub type Layer = Vec<DependentNode>;

//fn into_const_vec<T>(vec_ptr : &Vec<T>) -> ConstVec<T>
//{
//    return ConstVec::<T>{
//        len : vec_ptr.len().try_into().unwrap(), 
//        ptr : vec_ptr.as_ptr(), 
//        step : std::mem::size_of::<T>().try_into().unwrap(),
//    }
//

#[no_mangle]
extern "C" fn graph_to_layer(graph: &mut PartialOrderGraph, idx: u32) -> &Vec<(usize, Vec<usize>)> 
{
    return &graph[idx as usize];  
}

#[no_mangle]
extern "C" fn layer_to_dependent_node(layer: &mut Layer, idx: u32) -> &DependentNode 
{
    return &layer[idx as usize];  
}

#[no_mangle]
extern "C" fn dependent_node_to_dependencies(node: &mut DependentNode) -> *mut ConstVec<usize> 
{
    return Box::into_raw( 
            Box::new(
                ConstVec::<usize>{
            len : node.dependencies.len().try_into().unwrap(), 
            ptr : node.dependencies.as_ptr(), 
            step : std::mem::size_of::<usize>().try_into().unwrap(),
        }
        ));
}

#[no_mangle]
extern "C" fn dependencies_destroy(node: &mut ConstVec<usize>)
{
    unsafe {
        let _ = Box::from_raw(node);
    }
}



/*
 * pauli_tracker_partial_order_graph
 * Extracts the partial order graph from the pauli tracker and measurement map   
 */
#[no_mangle]
pub extern "C" fn pauli_tracker_partial_order_graph(pauli_tracker: &MappedPauliTracker) -> *mut PartialOrderGraph
{
    let graph = Box::into_raw(
        Box::new(
            induced_order::get_order(
                Iterable::iter_pairs(pauli_tracker.pauli_tracker.as_storage()),
                pauli_tracker.mapper.as_slice()
           )));
    return graph;
}

#[no_mangle]
extern "C" fn lib_pauli_tracker_graph_destroy(graph: *mut PartialOrderGraph) {
    unsafe {
        let _ = Box::from_raw(graph);
    }
}
