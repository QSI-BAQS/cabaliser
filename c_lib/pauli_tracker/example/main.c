#include "pauli_tracker.h"
#include "pauli_tracker_cliffords.h"

#include <stdio.h>

int main(void) {
    MappedPauliTracker *tracker = pauli_tracker_create(2);
    pauli_track_x(tracker, 0, 1);
    pauli_tracker_cx(tracker, 1, 0);     

    pauli_tracker_print(tracker);

    pauli_tracker_destroy(tracker);
}
