#ifndef MEASUREMENT_ORDER_H
#define MEASUREMENT_ORDER_H

#include "pauliracker.h"
#include "pauliracker_cliffords.h"

struct measurement_schedule;
struct measurement_schedule_layer;
struct dependencies;

// Measurement schedule as a series of layers
struct measurement_schedule
{
    struct n_layers;
    struct measurement_schedule_layer* layers; 
};


// One layer of the measurement schedule
struct measurement_schedule_layer 
{
    size n_qubits; 
    struct measurement_dependencies* dependencies;
};

// One measurement in the schedule
struct measurement_dependencies
{
    size qubit_index; // Measured qubit
    size* dependencies; // Pauli correction dependencies   
};

typedef struct measurement_schedule measurement_schedule_t;


#endif
