//
// Created by Paolo on 26/04/2022.
//

#include "Instance.h"
#include <iostream>
using namespace std;

Instance::Instance(const std::string& fileName){

    ifstream file(fileName);
    if(!file.is_open()){
        cout << "wrong instance path. check it:" << fileName << endl;
        exit(-1);
    }

	int size = fileName.size();
	instSeed = fileName[size-5] - '0';

	file >> i_nVertex >> i_nEdge;
    u = new int[i_nEdge];
    w = new int[i_nEdge];
    i_cost = new double[i_nEdge];
    for(int e=0; e < i_nEdge; e++){
        u[e] = -1;
        w[e] = -1;
        i_cost[e] = 0;
    }

    // read edge costs
    for(int e=0; e<i_nEdge; e++){
        file >> u[e] >> w[e] >> i_cost[e];
    }

}