#include <assert.h>

#define INSTRUCTIONS_TABLE

#include "widget.h"
#include "tableau_operations.h"
#include "input_stream.h"
#include "instructions.h"

#include "test_tableau.h"

#define ASSERT_SLICES_EQUAL(tab_a, tab_b, i) { \
assert(tab_a->slices_x[i][0] == tab->slices_x[i][0]); \
assert(tab_a->slices_z[i][0] == tab->slices_z[i][0]); \
assert(tab_a->phases[0] == tab_b->phases[0]);  \
}


void test_tableau_copy(const size_t n_qubits)
{
    widget_t* wid = widget_create(n_qubits, n_qubits);
    tableau_destroy(wid->tableau);
    wid->tableau = tableau_random_create(n_qubits);
    tableau_t* tab = tableau_copy(wid->tableau);

    for (size_t i = 0; i < sizeof(size_t); i++)
    {
        for (size_t j = 0; j < tab->slice_len / CHUNK_SIZE; j++)
        {
            assert(tab->slices_x[i][j] == wid->tableau->slices_x[i][j]);
            assert(tab->slices_z[i][j] == wid->tableau->slices_z[i][j]);
        }
    }
    for (size_t j = 0; j < tab->slice_len / CHUNK_SIZE; j++)
    {
        assert(tab->phases[j] == wid->tableau->phases[j]);
    }
    tableau_destroy(tab);
    widget_destroy(wid);
    return;
}


void test_identity(const size_t n_qubits)
{
    // 8 Qubit tableau
    widget_t* wid = widget_create(sizeof(size_t), sizeof(size_t));
    tableau_destroy(wid->tableau);
    wid->tableau = tableau_random_create(n_qubits);
    tableau_t* tab = tableau_copy(wid->tableau);

    instruction_stream_u inst;
    inst.single.opcode = _I_;
    inst.single.arg = 0;

    // Random starting opcode
    for (size_t i = 0; i < sizeof(size_t); i++)
    {
        // Apply the tableau update to the tableau
        SINGLE_QUBIT_OPERATIONS[inst.single.opcode ^ LOCAL_CLIFFORD_MASK](tab, i);

        parse_instruction_block(wid, &inst, 1);
        assert(wid->queue->table[i] == _I_);

        // Force application of local cliffords
        apply_local_cliffords(wid);

        ASSERT_SLICES_EQUAL(wid->tableau, tab, i);
    }
    tableau_destroy(tab);
    widget_destroy(wid);
    return;
}

void test_stream_X(const size_t n_qubits)
{
    widget_t* wid = widget_create(n_qubits, n_qubits);
    tableau_destroy(wid->tableau);
    wid->tableau = tableau_random_create(n_qubits);
    tableau_t* tab = tableau_copy(wid->tableau);

    instruction_stream_u inst;
    inst.single.opcode = _X_;

    // Random starting opcode
    for (size_t i = 0; i < n_qubits; i++)
    {
        inst.single.arg = i;

        // Apply the tableau update to the tableau
        SINGLE_QUBIT_OPERATIONS[inst.single.opcode ^ LOCAL_CLIFFORD_MASK](tab, i);

        parse_instruction_block(wid, &inst, 1);
        assert(wid->queue->table[i] == _X_);

        apply_local_cliffords(wid);

        ASSERT_SLICES_EQUAL(wid->tableau, tab, i);
    }
    tableau_destroy(tab);
    widget_destroy(wid);
    return;
}

void test_single_qubit_stream(const size_t n_qubits)
{

    widget_t* wid = widget_create(n_qubits, n_qubits);
    tableau_destroy(wid->tableau);
    wid->tableau = tableau_random_create(n_qubits);
    tableau_t* tab = tableau_copy(wid->tableau);

    instruction_stream_u inst;
    inst.single.opcode = _I_;

    // Random starting opcode
    uint8_t opcode = rand() % N_LOCAL_CLIFFORDS;
    for (size_t i = 0; i < wid->n_qubits; i++)
    {
        inst.single.arg = i;
        // New random opcode
        opcode = ((opcode + rand()) % N_LOCAL_CLIFFORD_INSTRUCTIONS) | LOCAL_CLIFFORD_MASK;

        // Update the opcode
        inst.single.opcode = opcode;
        inst.single.arg = i;

        // Apply the tableau update to the tableau
        SINGLE_QUBIT_OPERATIONS[opcode ^ LOCAL_CLIFFORD_MASK](tab, i);

        parse_instruction_block(wid, &inst, 1);


        assert(wid->queue->table[i] == opcode);

        //apply_local_cliffords(wid);

        //ASSERT_SLICES_EQUAL(wid->tableau, tab, i);
    }
    tableau_destroy(tab);
    widget_destroy(wid);
    return;
}

void test_cnot_stream(const size_t n_qubits)
{

    widget_t* wid = widget_create(n_qubits, n_qubits);
    tableau_destroy(wid->tableau);
    wid->tableau = tableau_random_create(n_qubits);
    tableau_t* tab = tableau_copy(wid->tableau);

    instruction_stream_u inst[3];
    inst[0].single.opcode = _X_;
    inst[1].single.opcode = _X_;
    inst[2].multi.opcode = _CNOT_;

    for (size_t i = 0; i < n_qubits - 1; i++)
    {
        ASSERT_SLICES_EQUAL(tab, wid->tableau, i);
        ASSERT_SLICES_EQUAL(tab, wid->tableau, i + 1);

        inst[0].single.arg = i;
        inst[1].single.arg = i + 1;
        inst[2].multi.ctrl = i;
        inst[2].multi.targ = i + 1;

        parse_instruction_block(wid, inst, 3);

        tableau_X(tab, i);
        tableau_X(tab, i + 1);
        tableau_CNOT(tab, i, i + 1);

        ASSERT_SLICES_EQUAL(tab, wid->tableau, i);
        ASSERT_SLICES_EQUAL(tab, wid->tableau, i + 1);
    }
    tableau_destroy(tab);
    widget_destroy(wid);
    return;
}


void test_cz_stream(const size_t n_qubits)
{
    widget_t* wid = widget_create(n_qubits, n_qubits);
    tableau_destroy(wid->tableau);
    wid->tableau = tableau_random_create(n_qubits);
    tableau_t* tab = tableau_copy(wid->tableau);

    instruction_stream_u inst[3];
    inst[0].single.opcode = _X_;
    inst[1].single.opcode = _X_;
    inst[2].multi.opcode = _CZ_;

    for (size_t i = 0; i < n_qubits - 1; i++)
    {
        ASSERT_SLICES_EQUAL(tab, wid->tableau, i);
        ASSERT_SLICES_EQUAL(tab, wid->tableau, i + 1);

        inst[0].single.arg = i;
        inst[1].single.arg = i + 1;
        inst[2].multi.ctrl = i;
        inst[2].multi.targ = i + 1;

        parse_instruction_block(wid, inst, 3);

        tableau_X(tab, i);
        tableau_X(tab, i + 1);
        tableau_CZ(tab, i, i + 1);

        ASSERT_SLICES_EQUAL(tab, wid->tableau, i);
        ASSERT_SLICES_EQUAL(tab, wid->tableau, i + 1);
    }
    tableau_destroy(tab);
    widget_destroy(wid);
    return;
}


int main()
{
    for (size_t n_qubits = 8; n_qubits < 1024; n_qubits+= 8) 
    {
        test_tableau_copy(n_qubits);
        test_identity(n_qubits);
        test_stream_X(n_qubits);

        test_single_qubit_stream(n_qubits);
        test_cnot_stream(n_qubits);
        test_cz_stream(n_qubits);
    }

    return 0;
}
