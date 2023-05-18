/// =========================================================================
/// Copyright (C) 2022 Saman Pordanesh (saman.pordanesh@ucalgary.ca)
/// All Rights Reserved.
/// =========================================================================

#include "find_deadlock.h"
#include "common.h"
#include "iostream"
#include <string>

class FastGraph {
public:
    std::vector<std::vector<int>> adj_list;
    std::vector<int> out_counts;
    std::vector<std::string> dictionary;
} fGraph;


std::vector<int> topologicalSort(FastGraph & graph){ // careful use & graph to avoid copy graph each time
    
    // final answer (all cycle process, if there is any)
    std::vector<int> procs;
    // a copy of all nodes with their outward edges count
    std::vector<int> out = graph.out_counts;

    // all processes (resources) which have 0 outgoing edges
    std::vector<int> zeros;
    // feeding loop
    for (size_t j = 0; j < out.size(); ++j) {
        if (out[j] == 0){
            zeros.push_back((int)j);
        }
    }

    // loop through all nodes with zero outgoing edges
    // we will add some during the process
    while (!zeros.empty()){
        int n = zeros[zeros.size() - 1];
        zeros.pop_back();
        for(auto n2 : graph.adj_list[n]){
            out[n2]--;
            if (out[n2] == 0){
                zeros.push_back(n2);
            }
        }
    }

    // finding all processes which involved in the cycle (if there is any)
    for (int j = 0; (size_t)j < out.size(); ++j) {
        if (out[j] > 0){
            procs.push_back(j);
        }
    }
    
    return procs; // return final vector
}


Result find_deadlock(const std::vector<std::string> & edges){

    Result result;

    FastGraph graph; // main graph
    std::vector<std::string> line; // all lines as entry
    int left, right; // left and right of an arrow (after converting to integers)
    std::vector<int> cycleProcs; //all processes which involve in a cycle
    Word2Int word2Int; // helper class to convert strings to integers 

    // main loop to loop through all edges 
    for (size_t i = 0; i < edges.size(); ++i) {

        // this is necessary to avoiding null ptr compilation error
        if (graph.out_counts.empty()){
            graph.out_counts.push_back(0);
        }
        if (graph.adj_list.empty()){
            graph.adj_list.push_back(std::vector<int>());
        }

        // splitting the line, based on free spaces
        line = split(edges[i]);

        // convert left(P) to an equivalent integer
        left = word2Int.get(line[0]);
        if (graph.dictionary.empty() && left == 0){
            graph.dictionary.push_back(line[0]);
        } else if ((size_t)left > graph.dictionary.size() - 1){
            // Put the original name of P in the dictionary
            // The index will be its equivalent integer
            graph.dictionary.push_back(line[0]);
        }

        // convert right(S) to an equivalent integer
        right = word2Int.get(line[2]+'*');
        if (graph.dictionary.empty() && right == 0){
            graph.dictionary.push_back(line[0]);
        } else if ((size_t)right > graph.dictionary.size() - 1){
            // Put the original name of S + '*' in the dictionary
            // The index will be its equivalent integer
            graph.dictionary.push_back(line[2]+'*');
        }

        
        // make place for our_counts vector for each new P or S 
        
        if ((size_t)left > graph.out_counts.size() - 1){
            graph.out_counts.push_back(0);
        }
        if ((size_t)right > graph.out_counts.size() - 1){
            graph.out_counts.push_back(0);
        }

        if ((size_t)left > graph.adj_list.size() - 1){
            graph.adj_list.push_back(std::vector<int>());
        }
        if((size_t)right > graph.adj_list.size() - 1){
            graph.adj_list.push_back(std::vector<int>());
        }


        // when we say P -> S
        if (line[1] == "->"){
            // S (right) has a toward arrow from P (left)
            graph.adj_list[right].push_back(left);
            // P (left) has an out-coming arrow
            graph.out_counts[left]++;
        } else if (line[1] == "<-"){
            // P (left) has a toward arrow from S (right)
            graph.adj_list[left].push_back(right);
            // S (right) has an out-coming arrow
            graph.out_counts[right]++;
        }
        
        // running the toposort for the current graph
        // returns the all processes involve a cycle (if there is any)
        cycleProcs = topologicalSort(graph);

        // if cycleProcs not empty, means there is a cycle
        if (!cycleProcs.empty()) {
            //convert integers to their equivalent process name
             for (int process : cycleProcs){
                 std::string proc = graph.dictionary[process];
                 if(proc.back() != '*'){ // avoiding consider a resource as a process 
                     result.procs.push_back(graph.dictionary[process]);
                 }
             }
            result.index = i; // record the index which cycle happened
            return result;
        }
    }
    result.index = -1; // return index -1 if there is no cycle in the graph
    return result;
}
