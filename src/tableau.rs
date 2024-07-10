//mod aligned_chunk;

#![feature(get_many_mut)]

use crate::aligned_chunk;

/*
 * Tableau
 * Tableau object consisting of two vectors of bitvectors
 * These vectors are of fixed size at compile time, however they may be rotated between row major
 * and column major order.
 *
 * The constructor for this tableau only requires a number of qubits
 * Implicitly as this tableau will only be used for graph state construction, it starts with all X entries * zeroed, and the Z block equal to the identity matrix
 *
 */
struct Tableau {
    x_component: Vec<aligned_chunk::AlignedChunks>,
    z_component: Vec<aligned_chunk::AlignedChunks>,
    phase_component: aligned_chunk::AlignedChunks, // TODO: Vec this for u64
    n_qubits: u64,
    col_major: bool,
}

impl Tableau {
    /*
     * Tableau::new
     * Constructor for the graph state tableau
     * :: n_qubits : u32 :: Takes a number of qubits
     *
     * There is the potential for this to silently fail if usize < u32 on the target architecture
     */
    // TODO: Break up if n_qubits > u32
    fn new(n_qubits: u64) -> Self {
        // X and Z Tableaus
        // This structure ensures that each vec may be quickly and independently queried
        // for fast full row or full column operations

        let mut x_component: Vec<aligned_chunk::AlignedChunks> =
            Vec::with_capacity(n_qubits as usize);
        let mut z_component: Vec<aligned_chunk::AlignedChunks> =
            Vec::with_capacity(n_qubits as usize);

        // Create each row
        for i in 0..n_qubits {
            x_component.push(aligned_chunk::AlignedChunks::new(n_qubits as usize));
            z_component.push(aligned_chunk::AlignedChunks::new_z(
                n_qubits as usize,
                i as usize,
            ));
        }

        let tableau = Self {
            x_component: x_component,
            z_component: z_component,
            phase_component: aligned_chunk::AlignedChunks::new(n_qubits as usize),
            n_qubits: n_qubits,
            col_major: true,
        };
        return tableau;
    }

    /*
     * cnot
     * Acts in place to implement a CNOT on the tableau representation
     *
     * self : Tableau :: Tableau object to apply the CNOT operator to
     * ctrl : u32     :: Control qubit of the CNOT
     * targ : u32     :: Target qubit of the CNOT
     */
    fn cnot(&mut self, ctrl: u64, targ: u64) {
        // Operation should only occur in col major order
        assert!(self.col_major);

        // Cannot set target equal to control
        assert_ne!(ctrl, targ);

        // COL SUM
        // X_t ^= X_c
        // Z_c ^= X_t

        // TODO convert to const
        for idx in 0..self.x_component[ctrl as usize].len() {

            let ptr : *mut aligned_chunk::AlignedChunks = self.x_component.as_mut_ptr();

            let chunk_ctrl = ptr.wrapping_add(ctrl as usize); 
            let chunk_targ = ptr.wrapping_add(targ as usize); 

            unsafe {
                (&mut*chunk_targ).xor(&*chunk_ctrl);
            }

            let ptr : *mut aligned_chunk::AlignedChunks = self.z_component.as_mut_ptr();

            let chunk_ctrl = ptr.wrapping_add(targ as usize); 
            let chunk_targ = ptr.wrapping_add(ctrl as usize); 

            unsafe {
                (&mut*chunk_targ).xor(&*chunk_ctrl);
            }
        }
    }

    fn hadamard_col(&mut self, targ: usize) {
        assert!(self.col_major);
    }

    fn phase(&mut self, targ: u32) {
        // COL SWAP
        // Phase_t ^= Z_t
        // Z_t ^= X_t
    }

    fn phase_dag(&mut self, targ: u32) {
        // COL SWAP
        // Z_t ^= X_t
        // Phase_t ^= Z_t
    }

    fn flip_order(&mut self) {
        // Tranpose X and Z
        self.col_major ^= true;
    }
}

//    /*
//     * get_Z
//     * Retrieves an element from the Z component of the tableau
//     */
//    fn get_Z(self, idx: u32, jdx: u32) -> bool
//    {
//        // TODO: Confirm NOT operator
//        if !self.col_major
//        {
//            idx ^= jdx
//            jdx ^= idx
//            idx ^= jdx
//        }
//
//        let chunk_idx : usize = jdx /  CACHE_LINE_SIZE_BITS!() as usize;
//        let bit_idx : usize = jdx % CACHE_LINE_SIZE_BITS!() as usize;
//        return self.Z[idx as usize][chunk_idx] & (1 << bit_idx);
//    }
//
//    /*
//     * get_X
//     * Retrieves an element from the Z component of the tableau
//     */
//    fn get_X(self, idx: u32, jdx: u32) -> bool
//    {
//        // TODO: Confirm NOT operator
//        if !self.col_major
//        {
//            idx ^= jdx
//            jdx ^= idx
//            idx ^= jdx
//        }
//
//        let chunk_idx = jdx /  CACHE_LINE_SIZE_BITS!();
//        let bit_idx = jdx % CACHE_LINE_SIZE_BITS!();
//        return self.X[idx as usize][chunk_idx] & (1 << bit_idx);
//    }
//
//
