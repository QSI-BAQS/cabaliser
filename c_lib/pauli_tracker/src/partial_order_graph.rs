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
}

type Dependents = Vec<usize>;

#[repr(C)]
pub struct DependentNode {
    index : u32,
    dependencies : Dependents,
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
 * :: index : u32 :: Layer index
 * Should be unsafe due to aliased reference?
 */
#[no_mangle]
extern "C" fn lib_pauli_graph_to_layer(graph: &mut PartialOrderGraph, index: u32) -> &Vec<(usize, Vec<usize>)> 
{
    return &graph[index as usize];  
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
extern "C" fn lib_pauli_dependent_qubit_idx(layer: *mut Layer, index: usize) -> usize {
    unsafe {
        return layer.as_ref().expect("REASON")[index].index as usize;
    }
}



#[no_mangle]
extern "C" fn lib_pauli_layer_to_dependent_node(layer: *mut Layer, idx: u32) -> *mut ConstVec<usize> 
{
    unsafe {
        let node = lib_pauli_dependent_node_to_dependencies(
            &mut (&mut*layer)[idx as usize]
        );
        return node;  
    };
}


/*
 * dependent_node_to_dependencies
 * Converts a dependent node into a dependencies object
 *
 */
#[no_mangle]
extern "C" fn lib_pauli_dependent_node_to_dependencies(node: &mut DependentNode) -> *mut ConstVec<usize> 
{
    return Box::into_raw( 
            Box::new(
                ConstVec::<usize>{
            len : node.dependencies.len().try_into().unwrap(), 
            ptr : node.dependencies.as_ptr(), 
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


/*
 * Temporary printing function
 */
#[no_mangle]
extern "C" fn lib_pauli_tracker_graph_print(graph: *mut PartialOrderGraph) {
    unsafe {
        println!("{:?}", graph.as_ref().expect("REASON"))
    }
}
