#ifndef GRAPH_H
#define GRAPH_H

struct graph_t
{
    size_t n_nodes; 
    struct graph_node_t* nodes; 
};

struct graph_node_t
{
    size_t vertex_label;
    size_t n_edges;
    struct graph_node_t* adjacent_members;
};

#endif
