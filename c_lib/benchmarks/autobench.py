import os
import sys
from time import perf_counter

n_iterations = 10
seed = "1"

def pretty_data(data, align: int = 5):
    '''
    Prints data in a slightly more readable format
    '''
    for param, results in data:
        print(f"---=[ {param} ]=---")
        for res in results:
            print(f"{str(res[0]).ljust(align)} | {res[1]}")

def run_benchmark(path: str, *args) -> float:
    child_pid = os.fork()
    runtime = 0

    if child_pid:
        runtime = perf_counter()
        os.wait()
        runtime = perf_counter() - runtime
    else:
        # Silence stdout
        os.execl(path, path, *args)

    return runtime


def main():
    # decomp - n_qubits, n_gates
    results = []
    curr_res = []
    # n_qubits = 2^10 - 2^14
    for qubit_exp in range(10, 14):
        # n_gates = 2^8 - 2^13
        for gate_exp in range(8, 13):
            time_total = 0
            
            for i in range(0, n_iterations):
                time_total += run_benchmark("decomp.out", str(2 ** qubit_exp), str(2 ** gate_exp), seed)

            curr_res.append((2 ** gate_exp, time_total / n_iterations))
        results.append((2 ** qubit_exp, curr_res))
        curr_res = []

    print("-----===[ decomp ]===-----")
    pretty_data(results)
    print()

    # input_stream - n_qubits, n_gates
    results = []
    curr_res = []
    # n_qubits = 2^10 - 2^14
    for qubit_exp in range(10, 14):
        # n_gates = 2^8 - 2^13
        for gate_exp in range(8, 13):
            time_total = 0

            for i in range(0, n_iterations):
                time_total += run_benchmark("input_stream.out", str(2 ** qubit_exp), str(2 ** gate_exp), seed)

            curr_res.append((2 ** gate_exp, time_total / n_iterations))
        results.append((2 ** qubit_exp, curr_res))
        curr_res = []

    print("-----===[ input_stream ]===-----")
    pretty_data(results)
    print()


    # qft - n_qubits
    results = []
    # n_qubits = 2^4 - 2^8
    for qubit_exp in range(4, 8):
        time_total = 0
        
        for i in range(0, n_iterations):
            time_total += run_benchmark("qft.out", str(2 ** qubit_exp))

        results.append((2 ** qubit_exp, time_total / n_iterations))

    print("-----===[ qft ]===-----")
    for res in results:
        print(f"{res[0]:<5} | {res[1]}")
    print()

    results = []
    # transverse_h - n_qubits
    # n_qubits = 2^5 - 2^9
    for qubit_exp in range(5, 9):
        time_total = 0
        
        for i in range(0, n_iterations):
            time_total += run_benchmark("transverse_h.out", str(2 ** qubit_exp), seed)

        results.append((2 ** qubit_exp, time_total / n_iterations))

    print("-----===[ transverse_h ]===-----")
    for res in results:
        print(f"{res[0]:<5} | {res[1]}")
    print()



if __name__ == "__main__":
    main()
