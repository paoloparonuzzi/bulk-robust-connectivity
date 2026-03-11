//
// Created by Paolo on 20/10/2023.
//

#include "Problem.h"

using namespace std;

void Problem::linearRelaxation(double timeLimit){


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

	for(auto e: edges)
		lpModel.addVar(0.0, 1.0, e.cost, GRB_CONTINUOUS, "z_" + to_string(e.u) + "_" + to_string(e.v));
	lpModel.update();

	// at list n-1 edges must be activated for each scenario
	for(int i = 0; i < nScenario; i++){
		for(int e = 0; e < nEdge; e++){
			if(!scenarioUnsafeEdge[i].count(e))
				expr += getZlp(e);
		}
		lpModel.addConstr(expr >= nVertex - 1, "Add_" + to_string(i));
		expr.clear();

		// each vertex must be visited in each scenario
		for(int v = 0; v < nVertex; v++){
			for(auto u: adj[v]){
				int e = edgeIndex(u, v);
				if(!scenarioUnsafeEdge[i].count(e))
					expr += getZlp(e);
			}
			lpModel.addConstr(expr >= 1, "V_" + to_string(v) + "_" + to_string(i));
			expr.clear();
		}
	}
	lpModel.set(GRB_IntParam_OutputFlag, 0);
	lpModel.set(GRB_DoubleParam_TimeLimit, timeLimit);
	lpModel.set(GRB_IntParam_Threads, 1);

	clock_t timeStart = clock();
	double checkTime = (double) (clock() - timeStart)/CLOCKS_PER_SEC;
	bool cutAdded = true;
	vector<double> zValLp(nEdge, 0);
	while(cutAdded && checkTime < timeLimit){
		cutAdded = false;
		lpModel.optimize();
		clock_t timeStartCut = clock();
		int nSol = lpModel.get(GRB_IntAttr_SolCount);
		if(nSol > 0){

			for(int e = 0; e < nEdge; e++)
				zValLp[e] = getZlp(e).get(GRB_DoubleAttr_X);

			shuffle(orderedScenario.begin(), orderedScenario.end(), rng);
			for(int j = 0; j < nScenario; j++){
				int i = orderedScenario[j];

				if(algorithm == 2){

					shuffle(destination.begin(), destination.end(), rng);
					for(int t: destination){
						for(int e = 0; e < nEdge; e++){
							getSubFlowZ(i, t, e).set(GRB_DoubleAttr_LB, zValLp[e]);
							getSubFlowZ(i, t, e).set(GRB_DoubleAttr_UB, zValLp[e]);
						}
						modelFlowSub[i][t].optimize();
						int nSubSol = modelFlowSub[i][t].get(GRB_IntAttr_SolCount);
						if(nSubSol == 0){
							for(auto e : edges){
								GRBConstr c1 = modelFlowSub[i][t].getConstrByName("Act1_" + to_string(e.id));
								double dualRay1 = c1.get(GRB_DoubleAttr_FarkasDual);
								if(dualRay1 != 0){
									double coeff = modelFlowSub[i][t].getCoeff(c1, getSubFlowZ(i, t, e.id));
									expr -= dualRay1*coeff*getZlp(e.id);
								}
								GRBConstr c2 = modelFlowSub[i][t].getConstrByName("Act2_" + to_string(e.id));
								double dualRay2 = c2.get(GRB_DoubleAttr_FarkasDual);
								if(dualRay2 != 0){
									double coeff = modelFlowSub[i][t].getCoeff(c2, getSubFlowZ(i, t, e.id));
									expr -= dualRay2*coeff*getZlp(e.id);
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
							lpModel.addConstr(expr >= -rhs, "gurobiCut_" + to_string(nAddedCut));
							expr.clear();
							nAddedCut++;
							cutAdded = true;
							break;
						}
					}
				}

				if(algorithm == 3){

					bool isInteger = true;
					for(auto e: edges)
					{
						if(zValLp[e.id] > tol && zValLp[e.id] < 1 - tol)
						{
							isInteger = false;
							break;
						}
					}
					if(isInteger)
					{
						components.clear();
						for(int v = 0; v < nVertex; v++)
							visited[v] = 0;

						int nComponent = 0;
						for(int v = 0; v < nVertex; v++){
							if(!visited[v]){
								component.clear();
								nComponent++;
								dfs(v, i, zValLp, nComponent);
								components.push_back(component);
							}
						}

						if(nComponent > 1){
							for(auto e: edges){
								if(visited[e.u] != visited[e.v] && !scenarioUnsafeEdge[i].count(e.id))
									expr += getZlp(e.id);
							}
							lpModel.addConstr(expr >= nComponent - 1, "spanCut_" + to_string(nAddedCut));
							expr.clear();
							nAddedCut++;
							cutAdded = true;
						}
					}
					else
					{
						vector<double> xBar(nEdge, 0);
						for(int v = 0; v < nVertex; v++){
							parent[v] = v;
							rank[v] = 0;
						}
						for(auto e: edges)
							if(zValLp[e.id] > tol && !scenarioUnsafeEdge[i].count(e.id))
								if(find(e.u) != find(e.v))
									xBar[e.id] =
										findMaxPossibleValue(e.id, xBar, zValLp, i);

						double check = 0;
						// double checkSol = 0.0;
						for(auto e: edges){
							if(find(e.u) != find(e.v) && !scenarioUnsafeEdge[i].count(e.id)){
								expr += getZlp(e.id);
								check += zValLp[e.id];
								// checkSol += solution[e.id];
							}
						}
						unordered_set<int> parentSet(parent.begin(), parent.end());
						int nComponent = (int) parentSet.size(); // parents.size();
						if(check < nComponent - 1 - tol){
							lpModel.addConstr(expr >= nComponent - 1, "spanCut_" + to_string(nAddedCut));
							expr.clear();
							nAddedCut++;
							cutAdded = true;
						}
						expr.clear();
					}
				}

				if(algorithm == 4){
					shuffle(destination.begin(), destination.end(), rng);
					for(int t: destination){
						double maxFlow = myGoldberg(t, zValLp, i);
						if(maxFlow < 1 - tol){
							for(auto e: edges){
								if(!scenarioUnsafeEdge[i].count(e.id))
									if(cut[e.u] && !cut[e.v] || cut[e.v] && !cut[e.u])
										expr += getZlp(e.id);
							}
							lpModel.addConstr(expr >= 1, "flowCut_" + to_string(nAddedCut));
							expr.clear();
							nAddedCut++;
							cutAdded = true;
							break;
						}
					}
				}
				if(cutAdded)
					break;
			}
		}
		callbackTime += (double) (clock() - timeStartCut)/CLOCKS_PER_SEC;
		checkTime = (double) (clock() - timeStart)/CLOCKS_PER_SEC;
	}
	double time = (double) (clock() - timeStart)/CLOCKS_PER_SEC;

	cout << "gurobiEffTime: " << time << endl;

	int optFound = 0;
	if(cutAdded == false)
		optFound = 1;

	int nVar = lpModel.get(GRB_IntAttr_NumVars);
	int nCon = lpModel.get(GRB_IntAttr_NumConstrs);
	int nSol = lpModel.get(GRB_IntAttr_SolCount);
	double objVal = 0.0;
	if(nSol > 0)
		objVal = lpModel.get(GRB_DoubleAttr_ObjVal);
	cout << "nVar: " << nVar << endl;
	cout << "nCon: " << nCon << endl;
	cout << "objVal: " << objVal << endl;
	outputLine += to_string(nVar) + "\t" + to_string(nCon) + "\t" + to_string(optFound) + "\t" + to_string(time)
	              + "\t-\t" + to_string(objVal) + "\t-\t-\t" + to_string(nAddedCut) + "\t" +
	              to_string(callbackTime);

	// lpModel.write("lpModel.lp");
}
