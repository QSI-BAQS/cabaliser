# Cabaliser

Cabaliser's `python` interface consists of;
- Operation Sequences, used to declare gates, and how qubits pass through them
- Widgets, used to load Operation Sequences as circuits, and decompose those circuits
- Local Simulator, a way to execute circuits


## Basics 

In general, a Cabaliser program will take the structure;

```
Create operations

Load operations into a widget

Decompose the widget

Inspect the decomposed result
            OR 
Simulate the resulting circuit
```

For example, to run Cabaliser for graph of the circuit
```
        ┌─┐
│q⟩─────┤H├──────⟩⟩
        └─┘
```
and inspect the result, you could do:

```python
from cabaliser import gates     # Provides the gates by name
from cabaliser.operation_sequence import OperationSequence
from cabaliser.widget import Widget


# Create an operation sequence, setting the maximum number of operations
# in the sequence to 10
ops = OperationSequence(10)

# Manually add a single Hadamard gate, applied to the input qubit with ID 0
ops.append(gates.H, 0)

# Create a widget, with 1 qubit by default, and an upper limit of 64 qubits
# Teleportation of inputs is disabled by default
wid = Widget(1, 64)

# Apply the operations to the widget
wid(ops)

# Decompose the widget and inspect the result
wid.decompose()

# See section Json below for information on how to interpret the JSON data
print(wid.json())
```


## Operation Sequences

The `OperationSequence` object provides a way to store a series of operations upon qubits.

An `OperationSequence` is initialised with `OperationSequence(n_instructions: int)`, where `n_instructions` provides an upper bound on the number of instructions present in the sequence.

Operations can be added to an `OperationSequence` with the `.append(opcode, *args)` method.
The opcode must be the code of a valid gate (see below), and the arguments consist of any arguments to be passed to that gate.  Instructions should be appended in the order in which they are to be carried out.

### Gates

Operation sequences are composed of gates. Each gate is referred to by an ID, and accepts some number of arguments. Gates typically accept one or more Qubit IDs, where a Qubit ID is the index of an input qubit (starting from `0`). These are to be declared by the user.

The legal gates are:

| Gate Name | Arguments | Description |
|-|-|-|
| `I` | `qubit_id` | The identity gate, to be applied to the given `qubit_id` |
| `X` | `qubit_id` | The X (NOT) gate, to be applied to the given `qubit_id` |
| `Y` | `qubit_id` | The Y gate, to be applied to the given `qubit_id` |
| `Z` | `qubit_id` | The Z gate, to be applied to the given `qubit_id` |
| `H` | `qubit_id` | The Hadamard gate, to be applied to the given `qubit_id` |
| `S` | `qubit_id` | The S gate, to be applied to the given `qubit_id` |
| `Sd` or `Sdag` or `R` | `qubit_id` | The S-Dagger gate, to be applied to the given `qubit_id` |
| `CNOT` | `control_id, target_id` | The CNOT gate, to be applied to the given qubit `target_id`, using `control_id` as the condition |
| `CZ` | `control_id, target_id` | The CZ gate, to be applied to the given qubit `target_id`, using `control_id` as the condition |
| `RZ` | `qubit_id, rotation_tag` | An arbitrary rotation in Z. `rotation_tag` describes which rotation is to be applied, and should be one of `0` (identity rotation), `1`, (T rotation), `2`, (T-Dagger rotation), <TODO - Check for other rotations>) |
| `MEAS` |  `qubit_id` | Performs a measurement of the given `qubit_id` | 
| `MCX` | `control_id, targets_id` | Performs a measurement of the qubit `control_id`, and uses the classical outcome to control an X gate applied to the qubit `target_id` |
| `MCY` | `control_id, targets_id` | Performs a measurement of the qubit `control_id`, and uses the classical outcome to control an Y gate applied to the qubit `target_id` |
| `MCZ` | `control_id, targets_id` | Performs a measurement of the qubit `control_id`, and uses the classical outcome to control an Z gate applied to the qubit `target_id` |

Note that `SWAP` and `TOFFOLI` gates are not provided, and are intended to be composed from the above gates. See the `examples` folder for some implementations of Toffoli gates.

## Widgets

A Widget provides a means of loading an OperationSequence, and decomposing it into a graph specifying the compiled quantum circuit.

At minimum, all that is required with a Widget is to create it (specifying some initial properties), load an OperationSequence, and then decompose the Widget into a graph.

### Creation

To create a widget, the number of initial qubits, and the maximum number of qubits, must be specified. Additional optional parameters can be provided.

```
Widget(n_qubits: int, n_qubits_max: int, ...)
```

Widgets are to be created with the following parameters:
| Parameter | Default | Description |
|-|-|-|
| `n_qubits : int` | N/A (required) | The number of initial qubits, the "width" of the Widget |
| `n_qubits_max : int` | N/A (required) | The maximum number of qubits within the entire Widget |
| `teleport_input : bool` | `False` | Whether the inputs should be teleported <TODO - Describe> | 
| `n_inputs : int` | `n_qubits` | Specifies the number of input qubits (defaults to `n_qubits`, the size of the qubit register). Only required if the number of inputs differs from the register size. |


### Usage

To consume an OperationSequence with a Widget, call that Widget on the desired OperationSequence - `<Widget Instance>(<OperationSequence Instance>)`.


### Decomposition and Inspection

Before inspection, a Widget must be decomposed <TODO - Describe what this does>. Once decomposed, the entire state of the Widget can be accessed (as JSON data), with `<Widget Instance>.json()` - see below for a description of each field. Individual pieces of data can be accessed with `get_measurement_tags`, `get_local_cliffords`, `get_io_map`, `get_adjacencies`, `get_pauli_corrections`, and `get_schedule`, which correspond to the below fields.


### JSON Fields

| Field | Type | Interpretation |
|-|-|-|
| `n_qubits` | `int` | The number of actual qubits present in the graph | 
| `statenodes` | `list<int>` | The IDs of the input qubit nodes |
| `adjacencies` | `dict<int><list<int>>` | A list of adjacencies within the DAG representation of the circuit, per node. Maps each node ID to a list of all nodes (by ID) adjacent to it|
| `local_cliffords` | `list<str>` | A list of Cliffords |
| `consumptionschedule` | `list<list<dict<int><list<int>>>>` | A list of lists of relationships between nodes. Maps each node to any nodes consumed by it. |
| `measurement_tags` | `list<int>` | The state of measurements for each node |
| `paulicorrections` | `list<dict<int><str>>` | A list of mappings between nodes and the Pauli Corrections applied to that node |
| `outputnodes` | `list<int>` | A list of nodes (by ID) that would be output by the graph |
| `time` | `int` | The time required for the graph - determined by the length of the consumption schedule |
| `space` | `int` | The space required for the graph |


## Simulation

Cabaliser can also perform local simulations of decomposed Widgets, and their JSON data.

The `local_simulator` module (`from cabaliser import local_simulator`) provides two interfaces for simulating Widgets, `simulate_widget`, and `simulate_dict_as_widget`.

These accept either a `Widget`, or a dictionary (the JSON acquired from a decomposed Widget). All further (optional) non-keyword arguments are interpreted as the input states.

Optional keyword arguments available are; <TODO - check kr and types>
- `input_state : tuple<kr thing>` - the input states, provided as a list. Takes priority over the non-keyword argument states.
- `table : dict<int><kr thing>` - a table mapping RZ tags to their corresponding values within the local simulator (see the example)
- `trace_graph : bool` - defaults to `True`. Determines whether unneeded qubits should be traced out of the graph state. 

These functions return a `numpy.array`, which can be output using `local_simulator.vec(<simulation result>)`.

A minimal example would be:
```
from cabaliser import gates
from cabaliser.operation_sequence import OperationSequence
from cabaliser.widget import Widget
from cabaliser import local_simulator

# RZ tags (declared for ease-of-use)
_I_ = 0
_T_ = 1
_Tdag_ = 2

# Provide a table describing the RZ tags
# (to be used by the simulator)
table = {
    _I_: local_simulator.I,
    _T_: local_simulator.T,
    _Tdag_: local_simulator.Tdag
}

# This code is a copy of what was done in "single_hadamard" 
ops = OperationSequence(10)

ops.append(gates.H, 0)

wid = Widget(1, 64)

wid(ops)

wid.decompose()

# Save the JSON to be passed to simulate_dict_as_widget
wid_json = wid.json()


# Create an input state, passing in a zero qubit
input_state = local_simulator.kr(
    local_simulator.zero_state
)

# Simulate the state of the Widget itself
widget_state = local_simulator.simulate_widget(
    wid,
    input_state=input_state, # Keyword argument must be used to pass in input_state as a tuple
    table=table # As above
)

# Simulate the state of Widget from the JSON output
json_widget_state = local_simulator.simulate_dict_as_widget(
    wid_json,
    input_state=input_state,
    table=table
)

# Output the two simulation results
print("Widget Simulation Result")
local_simulator.vec(widget_state)

print("Widget JSON Simulation Result")
local_simulator.vec(json_widget_state)
```
