//mod aligned_chunk;
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
    n_qubits: u64, 
    col_major: bool,
}

impl Tableau
{
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

        let mut x_component : Vec<aligned_chunk::AlignedChunks> = Vec::with_capacity(n_qubits as usize);
        let mut z_component : Vec<aligned_chunk::AlignedChunks> = Vec::with_capacity(n_qubits as usize); 

        // Create each row 
        for i in 0..n_qubits {
            x_component.push(aligned_chunk::AlignedChunks::new(0));
            z_component.push(aligned_chunk::AlignedChunks::new(0));
        }

        let tableau = Self {
            x_component: x_component,
            z_component: z_component,
            n_qubits: n_qubits,
            col_major: true,
        };
        return tableau;
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
//    /*
//     * cnot
//     * Acts in place to implement a CNOT on the tableau representation
//     * 
//     * self : Tableau :: Tableau object to apply the CNOT operator to  
//     * ctrl : u32     :: Control qubit of the CNOT
//     * targ : u32     :: Target qubit of the CNOT
//     */ 
//    fn cnot(&mut self, ctrl: u32, targ: u32) 
//    {
//        // Operation should only occur in col major order
//        assert!(self.col_major);
//    
//        // Cannot set target equal to control
//        assert_ne!(ctrl, targ);
//
//    
//        // COL SUM
//        // X_t ^= X_c
//        // Z_c ^= X_t     
//
//        let mut idx = 0;
//        while (idx < self.n_qubits)
//        { 
//            self.X[targ as usize].as_mut_bitslice() ^ self.X[ctrl as usize].as_mut_bitslice()
//
//             = .bitxor(self.X[targ as usize]);
//            self.Z[ctrl as usize] = self.Z[ctrl as usize].bitxor(self.Z[targ as usize]);
//            idx += 64;
//        }
//    }
//
//    fn hadamard(&mut self, targ: u32) 
//    {
//        // COL SWAP
//        // X_t ^= Z_t
//        // Z_t ^= X_t     
//        // X_t ^= Z_t     
//    }
//
//    fn phase(&mut self, targ: u32) 
//    {
//        // COL SWAP
//        // Phase_t ^= Z_t 
//        // Z_t ^= X_t     
//    }
//
//    fn phase_dag(&mut self, targ: u32) 
//    {
//        // COL SWAP
//        // Z_t ^= X_t     
//        // Phase_t ^= Z_t 
//    }
//
//   fn flip_order(&mut self)
//   {
//        // Tranpose X and Z 
//        self.col_major ^= true; 
//   } 
//}
