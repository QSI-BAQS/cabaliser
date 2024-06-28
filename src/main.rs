//use crate::aligned_chunk;
mod aligned_chunk;
mod tableau;


fn main() {

    // Check chunks are correctly sized
    let ac = aligned_chunk::AlignedChunks::new(8);
    assert_eq!(ac.len(), 1);

    let ac = aligned_chunk::AlignedChunks::new(63);
    assert_eq!(ac.len(), 1);

    let ac = aligned_chunk::AlignedChunks::new(64);
    assert_eq!(ac.len(), 2);

    let ac = aligned_chunk::AlignedChunks::new(127);
    assert_eq!(ac.len(), 2);

    let mut ac = aligned_chunk::AlignedChunks::new(128);
    assert_eq!(ac.len(), 3);
    
    // Check that correct values exist in the chunk after initialisation
    for chk in 0..ac.len() {
        let chunk = ac.get_chunk(chk); 
        for i in 0..chunk.len() {
            let val : u64 = ac.get_chunk(chk).get_bit(i); 
            assert_eq!(val, 0);
        }
    }

    // Check inv function
    ac.inv(); 
    for chk in 0..ac.len() {
        let chunk = ac.get_chunk(chk); 
        for i in 0..chunk.len() {
            let val : u64 = ac.get_chunk(chk).get_bit(i); 
            assert_eq!(val, 1 << i);
        }
    }

    // inv again and re-check
    ac.inv(); 
    for chk in 0..ac.len() {
        let chunk = ac.get_chunk(chk); 
        for i in 0..chunk.len() {
            let val : u64 = ac.get_chunk(chk).get_bit(i); 
            assert_eq!(val, 0);
        }
    }

    // XOR test
    // This forms the basis of the rowsum operator, CNOT, Hadamard and others   
    let mut ac = aligned_chunk::AlignedChunks::new(1024);
    let mut bc = aligned_chunk::AlignedChunks::new(1024);

    assert_eq!(ac.len(), 1024 / 64 + 1);

    
    // Check that correct values exist in the chunk after initialisation
    // ac: 0x00 0x00 0xff 0xff
    for i in 2..4 {
        ac.get_chunk(i).negate();
    }

    // bc: 0x00 0xff 0x00 x0ff
    for i in 0..4 {
        if (i % 2) == 1  
        {
            bc.get_chunk(i).negate();
        }
    }

    // Expected value: 0x00 0xff 0xff 0x00
    ac.xor(&bc);
    for i in 0..ac.get_chunk(0).len() {
        let val : u64 = ac.get_chunk(0).get_bit(i); 
        assert_eq!(val, 0);
        let val : u64 = ac.get_chunk(1).get_bit(i); 
        assert_eq!(val, 1 << i);
        let val : u64 = ac.get_chunk(2).get_bit(i); 
        assert_eq!(val, 1 << i);
        let val : u64 = ac.get_chunk(3).get_bit(i); 
        assert_eq!(val, 0);
    }

    // Expected value: 0x00 0x00 0xff 0xff
    ac.xor(&bc);
    for i in 0..ac.get_chunk(0).len() {
        let val : u64 = ac.get_chunk(0).get_bit(i); 
        assert_eq!(val, 0);
        let val : u64 = ac.get_chunk(1).get_bit(i); 
        assert_eq!(val, 0);
        let val : u64 = ac.get_chunk(2).get_bit(i); 
        assert_eq!(val, 1 << i);
        let val : u64 = ac.get_chunk(3).get_bit(i); 
        assert_eq!(val, 1 << i);
    }


    // Expected value: 0x00 0x00 0xff 0xff
    ac.partial_xor(&bc, 3);
    for i in 0..ac.get_chunk(0).len() {
        let val : u64 = ac.get_chunk(0).get_bit(i); 
        assert_eq!(val, 0);
        let val : u64 = ac.get_chunk(1).get_bit(i); 
        assert_eq!(val, 0);
        let val : u64 = ac.get_chunk(2).get_bit(i); 
        assert_eq!(val, 1 << i);
        let val : u64 = ac.get_chunk(3).get_bit(i); 
        assert_eq!(val, 0);
    }

    
    println!("Passed!");

}
