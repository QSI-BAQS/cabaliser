////mod consts;
//use crate::CACHE_LINE_SIZE;
//use crate::USIZE_CACHE_LINE;
use std::mem;
use std::ops;

const BITS_TO_BYTE: usize = 8;
const CACHE_LINE_SIZE_BYTES: usize = 64;
const CACHE_LINE_SIZE_BITS: usize = CACHE_LINE_SIZE * BITS_TO_BYTE;
const USIZE_CACHE_LINE: usize = CACHE_LINE_SIZE_BYTES / mem::size_of::<u64>();
const USIZE_U64: usize = 64;

/*
 * AlignedChunk
 * Cache line aligned chunk object
 */
#[repr(C, align(64))]
pub struct AlignedChunk {
    chunk: [u64; USIZE_CACHE_LINE],
}

// Provide your own size guarantees
pub struct IterCounterSizedMut<'a, T> {
    reference: &'a mut [T],
    counter: usize,
}

//impl<'a, T> Iterator for IterCounterSizedMut<'a, T>
//{
//    type Item = &'a T;
//    fn next(&mut self) -> Option<Self::Item> {
//        // End of iterator
//        if (self.counter >= self.reference.len()) {
//            return None;
//        }
//
//        // Return next item from iterator
//        self.counter += 1;
//        let value : Self::Item = &mut self.reference[self.counter];
//        return Some(value);
//    }
//}

impl AlignedChunk {
    pub fn new() -> Self {
        return Self {
            chunk: [0; USIZE_CACHE_LINE],
        };
    }

    pub fn iter_mut(&mut self) -> IterCounterSizedMut<u64> {
        return IterCounterSizedMut {
            reference: &mut self.chunk,
            counter: 0,
        };
    }

    pub fn negate(&mut self) {
        for i in 0..self.chunk.len() {
            self.chunk[i] = !self.chunk[i];
        }
    }

    fn xor(&mut self, other: &AlignedChunk) {
        for i in 0..self.chunk.len() {
            self.chunk[i] ^= other.chunk[i];
        }
    }

    pub fn len(&self) -> usize {
        return USIZE_CACHE_LINE;
    }

//    pub fn get(&mut self, idx: usize) -> &mut u64 {
//        assert!(idx < USIZE_CACHE_LINE);
//        return &mut self.chunk[idx];
//    }

    /*
     * get_bit
     * Returns a masked u64 chunk with the bit in the correct position
     */
    pub fn get_bit(&self, idx: usize) -> u64 {
        return ((1 << (idx % USIZE_CACHE_LINE)) & self.chunk[idx / USIZE_CACHE_LINE]);
    }
}

impl ops::BitXorAssign for AlignedChunk {
    fn bitxor_assign(&mut self, other: AlignedChunk) {
        for i in 0..self.chunk.len() {
            self.chunk[i] ^= other.chunk[i];
        }
    }
}
// TODO impl ops::Index
// And overwrite with the get method

/*
 * AlignedChunks
 * Cache line aligned chunk object
 * This is a vector wrapper around the aligned chunks
 * AlignedChunk enforces cache line alignment, while this struct collects and distributes functions
 * over the individual chunks.
 */
pub struct Tableau {
    chunks: Vec<mut AlignedChunk>,
    x_indices : Vec<mut usize>,
    z_indices : Vec<mut usize>,
}

impl Tableau {
    /*
     * new
     * Constructor for AlignedChunks
     * :: n_qubits : u64 :: Number of qubits to be managed by this object
     * Each qubit is managed by a single bit
     */
    pub fn new(n_qubits: usize) -> Self {
        let slice_size : usize = (n_qubits / CACHE_LINE_SIZE) ;  
        let mut aligned_chunks = Self {
            chunks: Vec::with_capacity(),
        };

        for _i in 0..(1 + n_qubits / CACHE_LINE_SIZE) {
            let mut tmp : AlignedChunk = AlignedChunk::new();
            aligned_chunks.chunks.push(tmp);
        }
        return aligned_chunks;
    }

    pub fn new_z(n_qubits: usize, idx: usize) -> Self {
        let mut aligned_chunks = Self::new(n_qubits);

        let chunk_idx = (idx / CACHE_LINE_SIZE);
        unsafe {
        (*aligned_chunks.chunks[chunk_idx]).chunk[(idx % CACHE_LINE_SIZE) / USIZE_U64] ^=
            (1 << (idx % USIZE_U64));
        }
        return aligned_chunks;
    }

    pub fn len(&self) -> usize {
        return self.chunks.len();
    }

    /*
     * xor
     * In place xor over two chunks
     */
    pub fn xor(&mut self, other: &AlignedChunks) {
        for i in 0..self.len() {
            unsafe {
                (*self.chunks[i]).xor(&*other.chunks[i]);
            }
        }
    }

    /*
     * partial_xor
     * For cases where we know that the first n terms may be ignored
     */
    pub fn partial_xor(&mut self, other: &AlignedChunks, idx: usize) {
        for i in idx..self.len() {
            unsafe {
                (*self.chunks[i]).xor(&*other.chunks[i]);
            }
        }
    }

    /*
     * inv
     * Flips all bits in a chunk
     */
    pub fn inv(&mut self) {
        for i in 0..self.len() {
            unsafe {
                (*self.chunks[i]).negate();
            }
        }
    }

//    pub fn get_chunk(&mut self, idx: usize) -> &mut AlignedChunk {
//        assert!(idx < self.len());
//        return &mut *self.chunks[idx];
//    }

//    pub fn swap_ref(&mut self, other: &mut Self) {
//        unsafe {
//            let tmp = &mut self.chunks;
//            self.chunks = other.chunks;
//            other.chunks = *tmp;
//        }
//    }
//
    //    pub fn iter_mut(&mut self) -> IterCounterSizedMut<AlignedChunk> {
    //        return IterCounterSizedMut::<AlignedChunk> {
    //            reference: &mut self.chunks,
    //            counter: 0,
    //        };
    //    }
}
