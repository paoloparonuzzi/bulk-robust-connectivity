#include <iostream>

#include "Instance.h"
#include "Problem.h"

using namespace std;

int main(int argc, char**argv){

    string fileName;
    double timeLimit;
    int algorithm;
	int lp;
	int nEdgeScenario;
    char*fileOutput;
    if(argc == 7){
        fileName = argv[1];
        timeLimit = atof(argv[2]);
        algorithm = atoi(argv[3]);
	    lp = atoi(argv[4]);
	    nEdgeScenario = atoi(argv[5]);
        fileOutput = argv[6];
    }
    else{
        cout << "Wrong number of parameter" << endl;
        return 1;
    }
	if(nEdgeScenario <= 0){
		cout << "At least one edge per scenario" << endl;
		return 1;
	}
	if(algorithm == 1 && lp == 1){
		cout << "Jump this combination" << endl;
		return 1;
	}

	cout << "fileName: " << fileName << endl;
    Instance instance(fileName);
    Problem problem(instance, algorithm, nEdgeScenario);

	if(nEdgeScenario > problem.nVertex){
		cout << "Maybe, too many edge per scenario" << endl;
		return 1;
	}

	int mstSol = problem.runKruskal();
	problem.buildBulkInstance();
	// problem.writeInstanceFile();
	// exit(1);
	double sProp = (double) nEdgeScenario/problem.nVertex;
	problem.outputLine = fileName + "\t" + to_string(problem.nVertex) + "\t" + to_string(problem.nEdge) + "\t"
			+ to_string(problem.nScenario) + "\t" + to_string(nEdgeScenario) + "\t" + to_string(sProp) +
			"\t" + to_string(mstSol) + "\t";

    if(algorithm == 0){
        problem.outputLine += "COMP\t" + to_string(lp)+"\t";
	    problem.runFlowModel(timeLimit, lp);
    }
    if(algorithm >= 2){
	    if(algorithm == 2)
		    problem.outputLine += "B-LP\t" + to_string(lp)+"\t";
	    if(algorithm == 3)
		    problem.outputLine += "B-span\t" + to_string(lp)+"\t";
	    if(algorithm == 4)
		    problem.outputLine += "B-flow\t" + to_string(lp)+"\t";
		if(lp)
			problem.linearRelaxation(timeLimit);
		else
			problem.runBenders(timeLimit);
    }

    problem.output.open(fileOutput, ios_base::app);
    if(problem.output.is_open()){
        problem.output << problem.outputLine << endl;
        problem.output.close();
    }
    return 1;
}
