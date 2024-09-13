#!/bin/bash

TIMEFMT="%U user %S system %P cpu %E total"
TIMEFORMAT=$TIMEFMT
CPU_VAR=$(lscpu | grep Model | head -n 1 | cut -d ':' -f2 | xargs echo)
FILE_NAME=benchmark_qubits_$CPU_VAR

touch "$FILE_NAME"

#TODO relative paths

GATE_COUNT=100
# Intentionally not putting in a variable here to get the variable counts printed to a file
for qubit_count in 10 100 1000 10000; do
    echo Benchmarking $qubit_count;
    for i in $(seq 1 10); do 
        var=$(time ../c/benchmarks/decomp.out $qubit_count $GATE_COUNT $i;) 2>&1 
#        echo qubits: $qubit_count gates: $gate_count   2>> "$FILE_NAME";
    done; 
done
