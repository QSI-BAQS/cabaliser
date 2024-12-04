-- Create the table
CREATE TABLE IF NOT EXISTS decorated_circuit(
    id INT PRIMARY KEY REFERENCES linked_circuit,
    layer INT
);

-- Stored procedures operating over the table

--
-- Reset each gate to unvisited 
--
CREATE OR REPLACE PROCEDURE unvisit_circuit()
LANGUAGE plpgsql
AS $$
BEGIN
UPDATE linked_circuit 
    SET visited = FALSE; 
COMMIT;
END;$$;


--
-- Read input nodes and create a decorated initial layer  
-- 
CREATE OR REPLACE PROCEDURE decorate_input()
LANGUAGE plpgsql
AS $$
BEGIN
-- Initial decorated circuit 
INSERT INTO decorated_circuit (
    SELECT id, 0 
    FROM linked_circuit 
    WHERE type = 'In'
        AND visited = FALSE);

UPDATE linked_circuit 
    SET visited = TRUE 
    WHERE linked_circuit.id = (
        SELECT decorated_circuit.id
        FROM decorated_circuit
        WHERE linked_circuit.id = decorated_circuit.id
    );

COMMIT;
END;$$;

--
-- Tag each gate with a layer number, layer by layer 
--
CREATE OR REPLACE PROCEDURE decorate_layer(
    layer_num int
    )
LANGUAGE plpgsql
AS $$
BEGIN
-- Decorate the next layer
INSERT INTO decorated_circuit (
    SELECT id, layer_num FROM linked_circuit 
    WHERE visited = FALSE 
        AND (prev_q1  / 10 = ( 
            SELECT id 
                FROM decorated_circuit 
                WHERE linked_circuit.prev_q1 / 10 = decorated_circuit.id))
        AND (prev_q2 IS NULL OR
            prev_q2 / 10 = (
            SELECT id 
                FROM decorated_circuit 
                WHERE linked_circuit.prev_q2 / 10 = decorated_circuit.id))
        AND (prev_q3 IS NULL OR
            prev_q3 / 10 = (
            SELECT id 
                FROM decorated_circuit 
                WHERE linked_circuit.prev_q3 / 10 = decorated_circuit.id))
    );
COMMIT;

-- Set the decorated nodes to visited 
UPDATE linked_circuit 
    SET visited = TRUE 
    WHERE linked_circuit.id = (
        SELECT decorated_circuit.id
        FROM decorated_circuit
        WHERE layer = layer_num
            AND linked_circuit.id = decorated_circuit.id
    );

-- Commit the changes
COMMIT;
END;$$;

--
-- Loop to decorate the whole circuit
--
CREATE OR REPLACE PROCEDURE decorate_circuit()
LANGUAGE plpgsql
AS $$
DECLARE
   layer integer := 1; 
BEGIN
-- Reset all nodes as unvisited
CALL unvisit_circuit();

-- Decorate the input nodes
CALL decorate_input();

-- Loop for each layer
WHILE EXISTS(SELECT NULL FROM linked_circuit WHERE visited = FALSE) LOOP
    CALL decorate_layer(layer);
    layer := layer + 1;
END LOOP;

-- Commit to be safe
COMMIT;
END;$$;
