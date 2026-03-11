//
// Created by Paolo on 29/06/2023.
//

#include "Problem.h"

using namespace std;

GRBVar Problem::getFlowF(int u, int v, int t, int i){
	return flowModel.getVarByName("f_" + to_string(u) + "_" + to_string(v) + "_"+ to_string(t) + "_"+  to_string(i));
}

void Problem::buildFlowModel(double timeLimit, int lp){

	for(int i = 0; i<nScenario; i++){
		for(int t=1; t<nVertex; t++){
			for(auto e : edges){
				flowModel.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS, "f_" + to_string(e.u) + "_" +
				                                                to_string(e.v) + "_" + to_string(t) + "_" + to_string(i));
				flowModel.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS, "f_" + to_string(e.v) + "_" +
				                                                to_string(e.u) + "_" + to_string(t) + "_" + to_string(i));
			}
		}
	}

	for(auto e : edges){
		if(lp)
			flowModel.addVar(0.0, 1.0, e.cost, GRB_CONTINUOUS, "z_" + to_string(e.u) + "_" + to_string(e.v));
		else
			flowModel.addVar(0.0, 1.0, e.cost, GRB_BINARY, "z_" + to_string(e.u) + "_" + to_string(e.v));
	}

	flowModel.update();

	for(int i = 0; i<nScenario; i++){
		for(int t=1; t<nVertex; t++){
			// for each terminal, at least 1 unit of flow
			for(int v=1; v<nVertex; v++)
				if(edgeIndex(0, v) != -1)
					expr += getFlowF(0, v, t, i);
			flowModel.addConstr(expr >= 1, "TerminalFlow_" + to_string(t) + "_" + to_string(i));
			expr.clear();

			// flow conservation
			for(int u=1; u<nVertex; u++){
				if(u != t){
					for(int v=0; v<nVertex; v++){
						if(v != u && edgeIndex(u, v) != -1){
							if(v != 0)
								expr += getFlowF(u, v, t, i) - getFlowF(v, u, t, i);
							else
								expr -= getFlowF(v, u, t, i);
						}
					}
					flowModel.addConstr(expr == 0, "FlowCons_" + to_string(u) + "_" + to_string(t) + "_" + to_string(i));
					expr.clear();
				}
			}

			// build activation constraints
			for(auto e : edges){
				flowModel.addConstr(getFlowF(e.u, e.v, t, i) <= getZc(e.id), "Act1_" + to_string(e.id) + "_" + to_string(t) + "_" + to_string(i));
				flowModel.addConstr(getFlowF(e.v, e.u, t, i) <= getZc(e.id), "Act2_" + to_string(e.id) + "_" + to_string(t) + "_" + to_string(i));
			}

			// forbid scenario edges
			for(auto e : scenarioUnsafeEdge[i]){
				getFlowF(edges[e].u, edges[e].v, t, i).set(GRB_DoubleAttr_UB, 0.0);
				getFlowF(edges[e].v, edges[e].u, t, i).set(GRB_DoubleAttr_UB, 0.0);
			}
		}

		// additional valid inequalities
		// at list n-1 edges must be activated for each scenario
		for(auto e : edges){
			if(!scenarioUnsafeEdge[i].count(e.id))
				expr += getZc(e.id);
		}
		flowModel.addConstr(expr >= nVertex - 1, "Add_"+to_string(i));
		expr.clear();

		// each vertex must be visited in each scenario
		for(int v = 0; v < nVertex; v++){
			for(auto u : adj[v]){
				int eId = edgeIndex(u, v);
				if(!scenarioUnsafeEdge[i].count(eId))
					expr += getZc(eId);
			}
			flowModel.addConstr(expr >= 1, "V_"+to_string(v)+"_"+to_string(i));
			expr.clear();
		}
	}

	flowModel.set(GRB_IntParam_OutputFlag, 0);
	flowModel.set(GRB_DoubleParam_TimeLimit, timeLimit);
	flowModel.set(GRB_IntParam_Threads, 1);
	flowModel.update();
	// flowModel.write("flowModel.lp");
	// exit(1);
}

void Problem::solveFlowModel(){
	try{
		flowModel.optimize();
		int nSol = flowModel.get(GRB_IntAttr_SolCount);
		if(nSol == 0){
			cout << "something wrong in gurobi compactModel" << endl;
			flowModel.computeIIS();
			flowModel.write("infcompactModel.ilp");
		}
	}
	catch(GRBException e) {
		cout << "Error code = " << e.getErrorCode() << endl;
		cout << e.getMessage() << endl;
	} catch(...) {
		cout << "Exception during optimization" << endl;
	}
}

/*void Problem::printCompactModelSolution(){
	try{
		cout << "Edge activated: " << endl;
		for(int e = 0; e < nEdge; e++){
			if(getComZ(e).get(GRB_DoubleAttr_X) > 0.001)
				cout << getComZ(e).get(GRB_DoubleAttr_X) <<  " (" << edges[e].u << ", " << edges[e].v << "); ";
		}
		cout << endl;
	}
	catch(GRBException e) {
		cout << "Error code = " << e.getErrorCode() << endl;
	}
}*/

void Problem::runFlowModel(double timeLimit, int lp){

	clock_t timeStart = clock();
	buildFlowModel(timeLimit, lp);
	double buildingTime = (double) (clock() - timeStart)/CLOCKS_PER_SEC;
	cout << "Model has been built in [s] " << buildingTime << endl;

	timeStart = clock();
	solveFlowModel();
	double time = (double) (clock() - timeStart)/CLOCKS_PER_SEC;
	cout << "gurobiEffTime: " << time << endl;

	int status = flowModel.get(GRB_IntAttr_Status);
	int optFound = 0;
	if(status == 2)
		optFound = 1;

	int nVar = flowModel.get(GRB_IntAttr_NumVars);
	int nCon = flowModel.get(GRB_IntAttr_NumConstrs);
	int nSol = flowModel.get(GRB_IntAttr_SolCount);
	double LB = 0.0;
	double UB = 1000.0;
	double gap = 100.0;
	int nNode = 0;
	if(nSol > 0){
		try{
			nNode = (int) round(flowModel.get(GRB_DoubleAttr_NodeCount));
			if(lp){
				UB = flowModel.get(GRB_DoubleAttr_ObjVal);
				LB = flowModel.get(GRB_DoubleAttr_ObjBound);
			}
			else{
				UB = round(flowModel.get(GRB_DoubleAttr_ObjVal));
				LB = round(flowModel.get(GRB_DoubleAttr_ObjBound));
			}
			gap = (UB - LB)*100.0/UB;
		}
		catch(GRBException e) {
			cout << "Error code = " << e.getErrorCode() << endl;
		}
		// printCompactModelSolution();
	}
	cout << "nVar: " << nVar << endl;
	cout << "nCon: " << nCon << endl;
	cout << "nNode: " << nNode << endl;
	cout << "UB: " << UB << endl;
	cout << "LB: " << LB << endl;
	cout << "gap: " << gap << endl;

	if(lp){
		outputLine += to_string(nVar) + "\t" + to_string(nCon) + "\t" + to_string(optFound) + "\t" + to_string(time)
		              + "\t-\t" + to_string(LB) + "\t-\t-";
	}
	else{
		outputLine += to_string(nVar) + "\t" + to_string(nCon) + "\t" + to_string(optFound) + "\t" + to_string(time)
		              + "\t" + to_string(UB) + "\t" + to_string(LB) + "\t" + to_string(gap) + "\t" + to_string(nNode);
	}
}