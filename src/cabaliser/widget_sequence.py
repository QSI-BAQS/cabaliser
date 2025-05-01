"""
Widget Sequence
"""

from cabaliser.widget import Widget
from cabaliser.operation_sequence import OperationSequence
from cabaliser import exceptions

class WidgetSequence:
    """
    Creates a sequence of widget objects
    Each widget has the same number of initial and max qubits
    """

    def __init__(self, qubit_width: int, max_qubits: int = 2000):
        """
        Initialiser for a widget sequence
        :: qubit_width : int :: Number of addressable qubits in each widget
        :: max_qubits : int :: Maximum number of qubits in each widget
        """
        if 2 * qubit_width >= max_qubits:
            raise exceptions.WidgetNotEnoughQubitsException(
                "Max qubits must be more than twice the number of initial qubits"
            )

        self.seq = []
        self._current_wid = None
        self._current_seq = None
        self._json = []
        self.rz_threshold = max_qubits - 2 * qubit_width
        self.qubit_width = qubit_width
        self.max_qubits = max_qubits

    def __call__(self, *args, json_output: bool = True, store_output = None, **kwargs):
        """
            Wrapper around widgetise_operation_sequence 
            That is ultimately a wrapper around _widgetise_operation_sequence_iter
            :: json_output  : bool :: Flag to trigger json output
        """
        if store_output is not None:  # Legacy syntax support
            json_output = store_output
        self.widgetise_operation_sequence(*args, json_output=json_output, **kwargs)

    def widgetise_operation_sequence(self, *args, **kwargs): 
        """
            Wrapper around _wigetise_operation_sequence_iter
            This function just forces the evaluation of the generator output 
        """ 
        return list(self._widgetise_operation_sequence_iter(*args, **kwargs))

    def _widgetise_operation_sequence_iter(
        self,
        ops: OperationSequence,
        progress: bool = False,
        json_output: bool = False,
        **widget_args
    ):
        """
        Processes an operations sequence
        :: ops : OperationSequence :: Sequence of operations to split and process
        :: progress : bool :: Simple progress printer
        :: json_output : bool :: Whether to yield json objects or widgets 
        :: **widget_args :: Args for the widget
            - rz_to_float=False
            - local_clifford_to_string=True 
        """
        ops_sequence = ops.split(self.rz_threshold)
        for i, seq in enumerate(ops_sequence):
            if progress:
                print(f"\r{i + 1} of {len(ops_sequence)}", flush=True, end="")

            wid = Widget(self.qubit_width, self.max_qubits)
            wid(seq)
            wid.decompose()

            if json_output:
                yield self._json.append(wid.json(**widget_args))
            else:
                yield wid

    def __iter__(self, *args, **kwargs):
        """
        Wrapper around _widgetise_operation_sequence_iter
        """
        return self._widgetise_operation_sequence_iter(*args, **kwargs)


    def json(self):
        """
        Returns the stored json objects for this sequence
        """
        return self._json
