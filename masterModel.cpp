//
// Created by Paolo on 04/05/2022.
//

#include "BendersCut.h"
#include "Problem.h"

using namespace std;

void Problem::runBenders(double timeLimit){

	if(algorithm == 2){
		modelFlowSub.resize(nScenario);
		for(int i = 0; i < nScenario; i++){
			// the first model (index 0) will be empty and not used
			for(int t = 0; t < nVertex; t++){
				modelFlowSub[i].emplace_back(env);
				if(t != 0)
					buildFlowSubProblem(i, t);
			}
		}
	}

	try{
		for(auto e : edges)
			masterModel.addVar(0.0, 1.0, e.cost, GRB_BINARY, "z_" + to_string(e.u) + "_" + to_string(e.v));

		masterModel.update();

		for(int i=0; i<nScenario; i++){
			// at list n-1 edges must be activated for each scenario
			for(int e = 0; e < nEdge; e++){
				if(!scenarioUnsafeEdge[i].count(e))
					expr += getZ(e);
			}
			masterModel.addConstr(expr >= nVertex - 1, "Add_" + to_string(i));
			expr.clear();

			// each vertex must be visited in each scenario
			for(int v = 0; v < nVertex; v++){
				for(auto u: adj[v]){
					int e = edgeIndex(u, v);
					if(!scenarioUnsafeEdge[i].count(e))
						expr += getZ(e);
				}
				masterModel.addConstr(expr >= 1, "V_" + to_string(v) + "_" + to_string(i));
				expr.clear();
			}
		}
		masterModel.set(GRB_IntParam_OutputFlag, 0);
		masterModel.set(GRB_DoubleParam_TimeLimit, timeLimit);
		masterModel.set(GRB_IntParam_Threads, 1);
		masterModel.set(GRB_DoubleParam_Heuristics, 0);
		masterModel.set(GRB_IntParam_LazyConstraints, 1);
		BendersCut lazyCallback = BendersCut(this);
		masterModel.setCallback(&lazyCallback);

		


		// masterModel.write("master.lp");

		clock_t timeStart = clock();
		masterModel.optimize();
		double time = (double) (clock() - timeStart)/CLOCKS_PER_SEC;

		cout << "gurobiEffTime: " << time << endl;

		int status = masterModel.get(GRB_IntAttr_Status);
		int optFound = 0;
		if(status == 2)
			optFound = 1;

		int nVar = masterModel.get(GRB_IntAttr_NumVars);
		int nCon = masterModel.get(GRB_IntAttr_NumConstrs);
		int nSol = masterModel.get(GRB_IntAttr_SolCount);
		double LB = 0.0;
		double UB = 1000.0;
		double gap = 100.0;
		int nNode = 0;
		if(nSol > 0){
			nNode = round(masterModel.get(GRB_DoubleAttr_NodeCount));
			UB = round(masterModel.get(GRB_DoubleAttr_ObjVal));
			LB = round(masterModel.get(GRB_DoubleAttr_ObjBound));
			gap = (UB - LB)*100.0/UB;

			/*for(int u = 0; u < nVertex; u++){
				for(int v = 0; v < nVertex; v++){
					if(u != v)
						cout << edges[edgeIndex(u, v)].cost << "\t";
					else
						cout << "0\t";
				}
				cout << endl;
			}
			cout << "Scenarios." << endl;
			for(auto scen : scenarioUnsafeEdge){
				for(auto e : scen)
					printEdge(e);
				cout << endl;
			}
			cout << "Edge activated: " << endl;
			int count = 0;
			for(auto e : edges){
				if(getZ(e.id).get(GRB_DoubleAttr_X) > 0.001){
					cout << getZ(e.id).get(GRB_DoubleAttr_X) << " (" << e.u << ", " << e.v << "); ";
					count++;
				}
			}
			cout << "count: " << count << endl;*/
		}
		cout << "nVar: " << nVar << endl;
		cout << "nCon: " << nCon << endl;
		cout << "nNode: " << nNode << endl;
		cout << "UB: " << UB << endl;
		cout << "LB: " << LB << endl;
		cout << "gap: " << gap << endl;

		outputLine += to_string(nVar) + "\t" + to_string(nCon) + "\t" + to_string(optFound) + "\t" + to_string(time)
		              + "\t" + to_string(UB) + "\t" + to_string(LB) + "\t" + to_string(gap) + "\t" + to_string(nNode)
		              + "\t" + to_string(nAddedCut) + "\t" + to_string(callbackTime);

		// masterModel.write("masterModel.lp");
	}
	catch(GRBException e){
		cout << "Error code = " << e.getErrorCode() << endl;
		cout << e.getMessage() << endl;
	}catch(...){
		cout << "Exception during optimization" << endl;
	}

}
