'''
    Constant values associated with gates
    This mirrors the contents of `lib/instruction_table.h
'''
from typing import Final
from ctypes import c_int8

# Gate constants
OPCODE_TYPE_MASK = 0xe0 
LOCAL_CLIFFORD_MASK: Final[c_int8] = 1 << 5
NON_LOCAL_CLIFFORD_MASK: Final[c_int8] = 1 << 6
RZ_MASK: Final[c_int8] = 1 << 7
CONDITIONAL_OPERATION_MASK = (1 << 6) | (1 << 7) 

# Set of legal gates
I: Final[c_int8] = 0x00 | LOCAL_CLIFFORD_MASK
X: Final[c_int8] = 0x01 | LOCAL_CLIFFORD_MASK
Y: Final[c_int8] = 0x02 | LOCAL_CLIFFORD_MASK
Z: Final[c_int8] = 0x03 | LOCAL_CLIFFORD_MASK
H: Final[c_int8] = 0x04 | LOCAL_CLIFFORD_MASK
S: Final[c_int8] = 0x05 | LOCAL_CLIFFORD_MASK
Sd: Final[c_int8] = 0x06 | LOCAL_CLIFFORD_MASK

# These are expected output values and may not be used as gates
_HX_: Final[c_int8] = 0x07 | LOCAL_CLIFFORD_MASK
_SX_: Final[c_int8] = 0x08 | LOCAL_CLIFFORD_MASK
_SdX_: Final[c_int8] = 0x09 | LOCAL_CLIFFORD_MASK
_HY_: Final[c_int8] = 0x0a | LOCAL_CLIFFORD_MASK
_HZ_: Final[c_int8] = 0x0b | LOCAL_CLIFFORD_MASK
_SH_: Final[c_int8] = 0x0c | LOCAL_CLIFFORD_MASK
_SdH_: Final[c_int8] = 0x0d | LOCAL_CLIFFORD_MASK
_HS_: Final[c_int8] = 0x0e | LOCAL_CLIFFORD_MASK
_HSd_: Final[c_int8] = 0x0f | LOCAL_CLIFFORD_MASK
_HSX_: Final[c_int8] = 0x10 | LOCAL_CLIFFORD_MASK
_HRX_: Final[c_int8] = 0x11 | LOCAL_CLIFFORD_MASK
_SHY_: Final[c_int8] = 0x12 | LOCAL_CLIFFORD_MASK
_SdHY_: Final[c_int8] = 0x13 | LOCAL_CLIFFORD_MASK
_HSH_: Final[c_int8] = 0x14 | LOCAL_CLIFFORD_MASK
_HRH_: Final[c_int8] = 0x15 | LOCAL_CLIFFORD_MASK
_SdHS_: Final[c_int8] = 0x16 | LOCAL_CLIFFORD_MASK
_SHSd_: Final[c_int8] = 0x17 | LOCAL_CLIFFORD_MASK

CNOT: Final[c_int8] = 0x00 | NON_LOCAL_CLIFFORD_MASK
CZ: Final[c_int8] = 0x01 | NON_LOCAL_CLIFFORD_MASK

RZ: Final[c_int8] = RZ_MASK

SINGLE_QUBIT_GATES = {
    I,
    X,
    Y,
    Z,
    H,
    S,
    Sd}

TWO_QUBIT_GATES = {CNOT, CZ}

SINGLE_QUBIT_GATE_TABLE = {
    I: "I",
    X: "X",
    Y: "Y",
    Z: "Z",
    H: "H",
    S: "S",
    Sd: "Sd",
    _HX_: "HX",
    _SX_: "SX",
    _SdX_: "SdX",
    _HY_: "HY",
    _HZ_: "HZ",
    _SH_: "SH",
    _SdH_: "SdH",
    _HS_: "HS",
    _HSd_: "HSd",
    _HSX_: "HSX",
    _HRX_: "HRX",
    _SHY_: "SHY",
    _SdHY_: "SdHY",
    _HSH_: "HSH",
    _HRH_: "HRH",
    _SdHS_: "SdHS",
    _SHSd_: "SHSd"
}
