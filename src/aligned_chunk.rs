////mod consts;
//use crate::CACHE_LINE_SIZE;
//use crate::USIZE_CACHE_LINE;
use std::mem;
use std::ops;

macro_rules! BITS_TO_BYTE {
    () => {
        8
    };
}

#[macro_export]
macro_rules! CACHE_LINE_SIZE {
    () => {
        64
    };
}

#[macro_export]
macro_rules! CACHE_LINE_SIZE_BITS {
    () => {
        CACHE_LINE_SIZE!() * BITS_TO_BYTE!()
    };
}

#[macro_export]
macro_rules! USIZE_CACHE_LINE {
    () => {
        CACHE_LINE_SIZE!() / mem::size_of::<usize>()
    };
}

/*
 * AlignedChunk
 * Cache line aligned chunk object
 */
#[repr(C, align(64))]
//#[derive(Debug, Copy, Clone)]
pub struct AlignedChunk {
    chunk: [usize; USIZE_CACHE_LINE!()],
}

// Provide your own size guarantees
pub struct IterCounterSized<'a, T: ?Sized> {
    reference : &'a mut T, 
    counter: usize,
    size : usize,
}

// Implements a Counter based iterator for array types [T]
impl<'a, T: Copy + ops::Index<usize, Output = T>> Iterator for IterCounterSized<'a, [T]>{
   type Item = T;  
   fn next(&mut self) -> Option<Self::Item>
   {
       // End of iterator 
        if (self.counter >= self.size) { 
            return None;
        }

        // Return next item from iterator
        self.counter += 1; 
        return Some(self.reference[self.counter]); 
   }
}

impl<'a, T: Copy + ops::Index<usize, Output = T>> Iterator for IterCounterSized<'a, Vec<T>>{
   type Item = T;  
   fn next(&mut self) -> Option<Self::Item>
   {
       // End of iterator 
        if (self.counter >= self.size) { 
            return None;
        }

        // Return next item from iterator
        self.counter += 1; 
        return Some(&self.reference[self.counter]); 
   }
}


impl AlignedChunk {
    pub fn new() -> Self {
        return Self {
            chunk: [0; USIZE_CACHE_LINE!()],
        };
    }

    pub fn iter(&mut self) -> IterCounterSized<[usize]> 
    {
        return IterCounterSized {
            reference : &mut self.chunk,  
            counter : 0,
            size : USIZE_CACHE_LINE!(), 
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

    pub fn get(&mut self, idx: usize) -> &mut usize {
        assert!(idx < USIZE_CACHE_LINE!());
        return &mut self.chunk[idx];
    }

    pub fn get_bit(&self, idx: usize) -> u8 {
        return ((1 << (idx % USIZE_CACHE_LINE!())) & self.chunk[idx / USIZE_CACHE_LINE!()]) as u8;
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
pub struct AlignedChunks {
    chunks: Vec<AlignedChunk>,
}

impl AlignedChunks {
    /*
     * new
     * Constructor for AlignedChunks
     * :: n_qubits : usize :: Number of qubits to be managed by this object
     * Each qubit is managed by a single bit
     */
    pub fn new(n_qubits: usize) -> Self {
        let mut aligned_chunks = Self {
            chunks: Vec::with_capacity(1 + n_qubits / CACHE_LINE_SIZE!()),
        };

        for _i in 0..(1 + n_qubits / CACHE_LINE_SIZE!()) {
            aligned_chunks.chunks.push(AlignedChunk::new());
        }
        return aligned_chunks;
    }

    pub fn len(&self) -> usize {
        return self.chunks.len();
    }

    pub fn xor(&mut self, other: &AlignedChunks) {
        for i in 0..self.len() {
            self.chunks[i].xor(&other.chunks[i]);
        }
    }

    pub fn inv(&mut self) {
        for i in 0..self.len() {
            self.chunks[i].negate();
        }
    }

    pub fn get_chunk(&mut self, idx: usize) -> &AlignedChunk {
        assert!(idx < self.len());
        return &self.chunks[idx];
    }

    pub fn iter(&mut self) -> IterCounterSized<Vec<AlignedChunk>> 
    {
        return IterCounterSized<Vec<AlignedChunk>> {
            reference : &mut self.chunks,  
            counter : 0,
            size : self.len(), 
        };
    }


}
