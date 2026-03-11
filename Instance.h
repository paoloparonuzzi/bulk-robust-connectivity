//
// Created by Paolo on 26/04/2022.
//

#ifndef BULKROBUSTMATCH_INSTANCE_H
#define BULKROBUSTMATCH_INSTANCE_H


#include <fstream>

struct Instance{

    // Problem data and constructor
    int i_nVertex{};
    int i_nEdge{};
    int* u;
    int* w;
    double* i_cost;
	int instSeed;

    explicit Instance(const std::string& fileName);

};


#endif //BULKROBUSTMATCH_INSTANCE_H
