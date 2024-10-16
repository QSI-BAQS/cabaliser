#include "lib_pauli_tracker.h"
#include <stdio.h>

int main(void) {
    MappedPauliTracker* live = lib_pauli_tracker_create(10);
   lib_pauli_tracker_destroy(live);
}
