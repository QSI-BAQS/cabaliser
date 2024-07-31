'''
    This file contains a set of proofs for the various optimisations on the tableau operations  
'''
from functools import reduce 

reductions = {  
    #00 01 10 11 
    0b0000 : '0', # Xll inputs lead to 0   
    0b1000 : 'NOT (X OR Z)', 
    0b0100 : '(NOT X) AND Z', 
    0b0010 : 'X AND (NOT Z)', 
    0b0001 : 'X AND Z', 
    0b1100 : 'NOT X', 
    0b0110 : 'X XOR Z', 
    0b0011 : 'X', 
    0b1010 : 'NOT Z', 
    0b0101 : 'Z', 
    0b1001 : '(NOT X) XOR Z', 
    0b0111 : 'X OR Z', 
    0b1011 : 'X OR (NOT Z)', 
    0b1101 : '(NOT X) OR Z', 
    0b1110 : 'NOT (X AND Z)', 
    0b1111 : '1'
}
table_inputs = ((0, 0), (0, 1), (1, 0), (1, 1))


def reduction(expr):
    global reductions
    global table_inputs

    # Phase is always an independent variable
    phase_reduction = lambda x: x[2] 
    x_reduction = lambda x: x[0] 
    z_reduction = lambda x: x[1] 


    vals = (map(
                lambda x: int(fn(expr(x[0], x[1]))),
                table_inputs) 
            for fn in (x_reduction, z_reduction, phase_reduction)
            ) 

    list_to_int = lambda lst : reduce(lambda x, y: x | y, map(lambda x: x[1] << (3 - x[0]), enumerate(lst)))  

    vals = list(map(list, vals))
    bin_vals = map(list_to_int, vals)

    return tuple(map(reductions.__getitem__, bin_vals))  


def H_(x, z, r=0):
    r ^= x & z
    return z, x, r

def S_(x, z, r=0):
    r ^= x & z
    z ^= x
    return x, z, r

def I_(x, z, r=0):
    return x, z, r


# Simple Instruction sequencer
# Composes left to right
def compose(*ops): 
    def _wrap(*args):
        for op in ops:
            args = op(*args)
        return args
    return _wrap

Z_ = compose(S_, S_)  
X_ = compose(H_, Z_, H_)  
Y_ = compose(X_, Z_)

R_ = compose(S_, S_, S_)

HX_ = compose(X_, H_) 
SX_ = compose(X_, S_) 
RX_ = compose(X_, R_) 
HY_ = compose(Y_, H_) 
HZ_ = compose(Z_, H_) 
SH_ = compose(H_, S_) 
RH_ = compose(H_, R_) 
HS_ = compose(S_, H_) 
HR_ = compose(R_, H_) 
HSX_ = compose(X_, S_, H_)
HRX_ = compose(X_, R_, H_)
SHY_ = compose(Y_, H_, S_)
RHY_ = compose(Y_, H_, R_)
HSH_ = compose(H_, S_, H_)
HRH_ = compose(H_, R_, H_)
RHS_ = compose(S_, H_, R_)
SHR_ = compose(R_, H_, S_)

