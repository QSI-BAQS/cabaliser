'''
schedule_footprint.
Calculates the memory footprint of a measurement schedule and graph state adjacencies
'''

from cabaliser.exceptions import ScheduleException
from cabaliser.pauli_tracker import PauliTracker

def schedule_footprint(
        adjacencies: list,
        schedule: list) -> int:
    '''
        Uses time optimal scheduler
        Does not check
    '''
    allocated = set()
    measured = set()

    max_footprint = 0

    for layer in schedule:
        # Allocate elements required for measurement
        for element in PauliTracker.qubit_indices(layer):
            allocated.add(element)
            for adj in adjacencies[element]:
                # If adjacency has already been measured don't re-add it
                if adj not in measured:
                    allocated.add(adj)

        # Update maximum footprint
        max_footprint = max(max_footprint, len(allocated))

        # Measure out elements in this layer of the schedule
        for element in PauliTracker.qubit_indices(layer):
            allocated.remove(element)
            measured.add(element)

    if not sum(map(len, schedule)) == len(measured):
        raise ScheduleException

    return max_footprint
