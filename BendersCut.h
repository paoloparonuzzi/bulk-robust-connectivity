//
// Created by Paolo on 25/11/2020.
//

#ifndef GRAPHPART_GRBCALLBACK_H
#define GRAPHPART_GRBCALLBACK_H


#include "Problem.h"

using namespace std;

class BendersCut : public GRBCallback{
public :

    Problem*problem;
	vector<double> zVal;
    explicit BendersCut(Problem*problemRef);

protected :
    void callback() override;

	friend struct Problem;

};


#endif //GRAPHPART_GRBCALLBACK_H
