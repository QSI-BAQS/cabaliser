use pauli_tracker::{
    tracker::{frames::induced_order},
    collection::Iterable,
};

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
pub struct OpaqueVec<'a,T>(&'a Vec<T>);
impl <T> OpaqueVec<T> {
    fn into_const_vec(self) -> ConstVec<T>
    {
        return ConstVec<T>{
            len : self.0.len(), 
            ptr : self.0.as_ptr(), 
        }
    }
}

type Dependents = OpaqueVec<usize>;

#[repr(C)]
pub struct DependentNode {
    index : u32,
    dependencies : Dependents,
}

pub struct Layer(OpaqueVec<DependentNode>);

type PartialOrderGraph = ConstVec<Layer>;



#[no_mangle]
extern "C" fn pauli_tracker_dependents_to_const_vec(dependents : Depentents) -> ConstVec<usize>
{
    return dependents.0.into_const_vec();
}

/*
 *
 *
 */
#[no_mangle]
extern "C" fn pauli_tracker_layer_to_const_vec(layer : Layer) -> ConstVec<DependentNode>
{
    return layer.0.into_const_vec();
}


/*
 * pauli_tracker_layer_to_const_vec
 * Maps a layer from a Rust native vector to a C struct  
 * :: layer : Layer :: The layer to map 
 * Returns a ConstVec<DependentNode> object
 */
#[no_mangle]
extern "C" fn pauli_tracker_layer_to_const_vec(layer : Layer) -> ConstVec<DependentNode>
{
    return layer.0.into_const_vec();
}


/*
 * pauli_tracker_partial_order_graph
 * Extracts the partial order graph from the pauli tracker and measurement map   
 * TODO: Convert the output to the C format before returning 
 */
#[no_mangle]
extern "C" fn pauli_tracker_partial_order_graph(pauli_tracker: &mut MappedPauliTracker) -> PartialOrderGraph
{
    graph = induced_order::get_order(
        Iterable::iter_pairs(pauli_tracker.pauli_tracker.as_storage()),
        pauli_tracker.mapper.as_slice()
    );
    
    return graph;
}
