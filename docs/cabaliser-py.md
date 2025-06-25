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

For example, to run Cabaliser for the circuit
``` <TODO - Format with nice ASCII>
         /-\ 
|q> -----|H|------>>
         \-/
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

# Create a widget <TODO - Describe n_qubits and max_qubits>
# Teleportation of inputs is disabled by default
wid = Widget(1, 64)

# Apply the operations to the widget
wid(ops)

# Decompose the widget and inspect the result
wid.decompose()

# See section Json below for information on how to interpret
# this
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
| `RZ` | `qubit_id, rotation_tag` | An arbitrary rotation in Z. `rotation_tag` describes which rotation is to be applied, and should be one of `0` (identity rotation), `1`, (T rotation), `2`, (T-Dagger rotation), <TODO - Check for other rotations) |
| `MEAS` |  `qubit_id` | Performs a measurement of the given `qubit_id` | 
| `MCX` | `control_id, targets_id` | <TODO> |
| `MCY` | `control_id, targets_id` | <TODO> |
| `MCZ` | `control_id, targets_id` | <TODO> |


## Widgets

A Widget provides a means of loading an OperationSequence, and decomposing it into a graph specifying the compiled quantum circuit <TODO - Reword>.

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
| `teleport_input : bool` | False | Whether the inputs should be teleported <TODO - Describe> | 
| `n_inputs : int` | `n_qubits` | Specifies the number of input qubits (defaults to `n_qubits`, the size of the qubit register). Only required if the number of inputs differs from the register size. |


### Decomposition and Inspection

Before inspection, a Widget must be decomposed <TODO - Describe what this does>. Once decomposed, the entire state of the Widget can be accessed (as JSON data), with `<Widget>.json()`.

