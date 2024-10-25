use pauli_tracker::{
    tracker::{frames::induced_order::{self}},
    collection::Iterable,
};
pub use pauli_tracker::tracker::{frames::induced_order::{PartialOrderGraph}};

use crate::const_vec::{
    ConstVec, vec_to_const_vec, 
};

use super::mapped_pauli_tracker::{
    MappedPauliTracker,
};


type Dependents = Vec<usize>;

#[repr(C)]
pub struct DependentNode {
    pub index : usize,
    pub dependencies : Dependents,
}

pub type Layer = Vec<DependentNode>;

/*
 * Layer operations
 * Each layer represents a set of simultaneously
 * schedulable operations 
 * Each element in the layer is a DependentNode 
 */
/*
 * graph_to_layer
 * :: graph : &mut PartialOrderGraph :: Partial order graph object
 * :: index : usize :: Layer index
 * Should be unsafe due to aliased reference?
 */
#[no_mangle]
extern "C" fn lib_pauli_graph_to_layer(graph: &mut PartialOrderGraph, index: usize) -> &Vec<(usize, Vec<usize>)> 
{
    return &graph[index];  
}

/*
 * lib_pauli_n_dependents
 * Wrapper function due to indeterminate structure of rust objects 
 * :: layer : *mut Layer :: Layer pointer  
 * Returns the number of elements in the layer
 */
#[no_mangle]
extern "C" fn lib_pauli_n_dependents(layer: *mut Layer) -> usize {
    unsafe {
        return layer.as_ref().expect("REASON").len();
    }
}

/*
 * lib_pauli_dependent_qubit_idx
 * Returns the qubit index of a dependent node   
 * :: layer : *mut Layer :: Layer pointer  
 * :: index : usize :: Index to query 
 * Returns the number of elements in the layer
 */
#[no_mangle]
extern "C" fn lib_pauli_dependent_qubit_idx(
    layer: *mut Layer,
    index: usize) -> usize {
    unsafe {
        return layer.as_ref().expect("REASON")[index].index;
    }
}

#[no_mangle]
extern "C" fn lib_pauli_layer_to_dependent_node(
    layer: *mut Layer,
    idx: usize) -> *mut ConstVec<usize> 
{
    unsafe {
        let node = vec_to_const_vec::<usize>(
            &mut (&mut*layer)[idx].dependencies
        );
        return node;  
    };
}

/*
 * dependent_node_to_dependencies
 * Converts a dependent node into a dependencies object
 */
#[no_mangle]
extern "C" fn lib_pauli_dependent_node_to_dependencies(node: &mut DependentNode) -> *mut ConstVec<usize> 
{
    return Box::into_raw( 
            Box::new(
                ConstVec::<usize>{
                    ptr : node.dependencies.as_mut_ptr(), 
                    len : node.dependencies.len(), 
                    cap : node.dependencies.capacity(), 
                }
            )
        );
}

#[no_mangle]
extern "C" fn dependencies_destroy(node: &mut ConstVec<usize>)
{
    unsafe {
        let _ = Box::from_raw(node);
    }
}

/*
 * lib_pauli_tracker_partial_order_graph
 * Extracts the partial order graph from the pauli tracker and measurement map   
 */
#[no_mangle]
pub extern "C" fn lib_pauli_tracker_partial_order_graph(pauli_tracker: &MappedPauliTracker) -> *mut PartialOrderGraph
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
extern "C" fn lib_pauli_n_layers(graph: *mut PartialOrderGraph) -> usize {
    unsafe {
        return graph.as_ref().expect("REASON").len();
    }
}

#[no_mangle]
extern "C" fn lib_pauli_tracker_graph_destroy(graph: *mut PartialOrderGraph) {
    unsafe {
        let _ = Box::from_raw(graph);
    }
}


#[no_mangle]
extern "C" fn lib_pauli_tracker_const_vec_destroy(obj: *mut ConstVec<usize>) {
    unsafe {
        let _ = Box::from_raw(obj);
    }
}

/*
 * Temporary printing function
 */
#[no_mangle]
extern "C" fn lib_pauli_tracker_graph_print(graph: *mut PartialOrderGraph) {
    unsafe {
        println!("{:?}", graph.as_ref().expect("REASON"))
    }
}
