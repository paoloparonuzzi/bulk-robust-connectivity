//
// Created by Paolo on 05/05/2022.
//

#include "Problem.h"

using namespace std;

void Problem::buildFlowSubProblem(int i, int t){

	for(auto e : edges){
		modelFlowSub[i][t].addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS,
		                          "fSub_" + to_string(e.u) + "_" + to_string(e.v));
		modelFlowSub[i][t].addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS,
		                          "fSub_" + to_string(e.v) + "_" + to_string(e.u));
	}
	for(auto e : edges)
		modelFlowSub[i][t].addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, "zSub_" + to_string(e.id));
	modelFlowSub[i][t].update();

	for(auto v : adj[0])
		expr += getSubF(0, v, t, i);
	// modelFlowSub[i][t].setObjective(expr, GRB_MAXIMIZE);
	modelFlowSub[i][t].addConstr(expr >= 1, "Feasibility");
	expr.clear();

	// flow conservation
	for(int u=1; u<nVertex; u++){
		if(u != t){
			for(auto v : adj[u]){
				if(v != 0)
					expr += getSubF(u, v, t, i) - getSubF(v, u, t, i);
				else
					expr -= getSubF(v, u, t, i);
			}
			modelFlowSub[i][t].addConstr(expr == 0, "FlowCons_" + to_string(u));
			expr.clear();
		}
	}

	// build activation constraints
	for(auto e : edges){
		modelFlowSub[i][t].addConstr(getSubF(e.u, e.v, t, i) <= getSubFlowZ(i, t, e.id), "Act1_" + to_string(e.id));
		modelFlowSub[i][t].addConstr(getSubF(e.v, e.u, t, i) <= getSubFlowZ(i, t, e.id), "Act2_" + to_string(e.id));
	}

	// forbid scenario edges
	for(auto e : scenarioUnsafeEdge[i]){
		getSubF(edges[e].u, edges[e].v, t, i).set(GRB_DoubleAttr_UB, 0.0);
		getSubF(edges[e].v, edges[e].u, t, i).set(GRB_DoubleAttr_UB, 0.0);
	}

	modelFlowSub[i][t].set(GRB_IntParam_OutputFlag, 0);
	modelFlowSub[i][t].set(GRB_DoubleParam_TimeLimit, 600);
	modelFlowSub[i][t].set(GRB_IntParam_Threads, 1);
	modelFlowSub[i][t].set(GRB_IntParam_InfUnbdInfo, 1);
	modelFlowSub[i][t].update();
}

bool Problem::gurobiFeasibilityCut(int i, vector<double>& zVal){

	shuffle(destination.begin(), destination.end(), rng);
	for(int t : destination){
		// for(int t = 1; t < nVertex; t++){
		for(int e = 0; e < nEdge; e++){
			getSubFlowZ(i, t, e).set(GRB_DoubleAttr_LB, zVal[e]);
			getSubFlowZ(i, t, e).set(GRB_DoubleAttr_UB, zVal[e]);
		}
		modelFlowSub[i][t].optimize();
		int nSol = modelFlowSub[i][t].get(GRB_IntAttr_SolCount);
		if(nSol == 0){
			for(int e = 0; e < nEdge; e++){
				GRBConstr c1 = modelFlowSub[i][t].getConstrByName("Act1_" + to_string(e));
				double dualRay1 = c1.get(GRB_DoubleAttr_FarkasDual);
				if(dualRay1 != 0){
					double coeff = modelFlowSub[i][t].getCoeff(c1, getSubFlowZ(i, t, e));
					expr -= dualRay1*coeff*getZ(e);
				}
				GRBConstr c2 = modelFlowSub[i][t].getConstrByName("Act2_" + to_string(e));
				double dualRay2 = c2.get(GRB_DoubleAttr_FarkasDual);
				if(dualRay2 != 0){
					double coeff = modelFlowSub[i][t].getCoeff(c2, getSubFlowZ(i, t, e));
					expr -= dualRay2*coeff*getZ(e);
				}
			}

			int nCon = modelFlowSub[i][t].get(GRB_IntAttr_NumConstrs);
			double rhs = 0.0;
			for(int c = 0; c < nCon; c++){
				GRBConstr con = modelFlowSub[i][t].getConstr(c);
				double dualRay = con.get(GRB_DoubleAttr_FarkasDual);
				if(dualRay != 0)
					rhs += con.get(GRB_DoubleAttr_RHS)*dualRay;
			}

			expr += rhs;
			return true;
		}
	}
	return false;
}
