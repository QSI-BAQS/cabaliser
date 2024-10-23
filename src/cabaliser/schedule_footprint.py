import copy

def schedule_footprint(
        adjacencies: list,
        schedule: list) -> int:
    '''
        Uses time optimal scheduler 
        Does not check correctness
    '''
    allocated = set() 
    measured = set()

    max_footprint = 0 

    qubit_index = lambda x: next(iter(x))  

    # initial allocation
    for layer in schedule:
        for element in map(qubit_index, layer):
            allocated.add(element)
            for adj in adjacencies[element]:
                if adj not in measured:
                    allocated.add(adj)
        max_footprint = max(max_footprint, len(allocated))
        for element in map(qubit_index, layer):
            allocated.remove(element)
            measured.add(element)

    try: 
        assert(sum(map(len, schedule)) == len(measured)) 
    except AssertionError:
        raise Exception("Schedule does not consume widget")

    return max_footprint
