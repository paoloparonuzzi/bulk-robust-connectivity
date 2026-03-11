//
// Created by Paolo on 19/07/2023.
//

#include "Problem.h"


using namespace std;

void Problem::dfs(int v, int i, std::vector<double>& zVal, int countComp){
	component.insert(v);
	visited[v] = countComp;
	for(auto u : adj[v]){
		int e = edgeIndex(u, v);
		if(!visited[u] && !scenarioUnsafeEdge[i].count(e) && zVal[e] > 0.5)
			dfs(u, i, zVal, countComp);
	}
}

bool Problem::buildIntFeasibilityCut(int i, std::vector<double>& zVal){

	if(algorithm == 4){
		shuffle(destination.begin(), destination.end(), rng);
		for(int t : destination){
			double maxFlow = myGoldberg(t, zVal, i);
			if(maxFlow < 1 - tol){
				for(auto e : edges){
					if(!scenarioUnsafeEdge[i].count(e.id)){
						if(cut[e.u] && !cut[e.v] || cut[e.v] && !cut[e.u])
							expr += getZ(e.id);
					}
				}
				expr -= 1;
				return true;
			}
		}
	}

	if(algorithm == 3){
		components.clear();
		for(int v = 0; v < nVertex; v++)
			visited[v] = 0;

		int nComponent = 0;
		for(int v = 0; v < nVertex; v++){
			if(!visited[v]){
				component.clear();
				nComponent++;
				dfs(v, i, zVal, nComponent);
				components.push_back(component);
			}
		}

		if(nComponent > 1){
			for(auto e: edges){
				if(visited[e.u] != visited[e.v] && !scenarioUnsafeEdge[i].count(e.id))
					expr += getZ(e.id);
			}
			expr -= (nComponent - 1);
			return true;
		}
	}
	return false;
}

double Problem::findMaxPossibleValue(int eId, vector<double>& xBar, vector<double>& zVal, int i){

	// double alpha = nVertex - 1;

	vector<double> d(nVertex, 2.0);
	for(int v=0; v<nVertex; v++){
		for(auto u : adj[v])
			d[v] -= xBar[edgeIndex(u,v)];
	}

	for(int e = 0; e < nEdge; e++){
		lemon_capacity_1[lemon_arcs_1[e]] = xBar[e];
		lemon_capacity_1[lemon_arcs_1[e + nEdge]] = xBar[e];
	}
	for(int e = 2*nEdge; e < 2*nEdge + nVertex; e++){
		if(d[e - 2*nEdge] > 0)
			lemon_capacity_1[lemon_arcs_1[e]] = d[e - 2*nEdge];
		else
			lemon_capacity_1[lemon_arcs_1[e]] = 0;

	}
	for(int e = 2*nEdge + nVertex; e < 2*nEdge + 2*nVertex; e++){
		if(d[e - (2*nEdge + nVertex)] < 0)
			lemon_capacity_1[lemon_arcs_1[e]] = -d[e - (2*nEdge + nVertex)];
		else
			lemon_capacity_1[lemon_arcs_1[e]] = 0;
	}

	// forbid scenario edges
	for(auto e: scenarioUnsafeEdge[i]){
		lemon_capacity_1[lemon_arcs_1[e]] = 0;
		lemon_capacity_1[lemon_arcs_1[e + nEdge]] = 0;
	}
	// make infinity arcs of current edge
	lemon_capacity_1[lemon_arcs_1[eId]] = DBL_MAX;
	lemon_capacity_1[lemon_arcs_1[eId+nEdge]] = DBL_MAX;
	lemon_capacity_1[lemon_arcs_1[2*nEdge + nVertex + edges[eId].v]] = DBL_MAX;

	double c = 0;
	for(int v = 0; v < nVertex; v++)
		if(d[v] < 0)
			c += d[v];

	double maxFlow = myGoldberdAux();
	double alpha = -1 + 0.5*(c+maxFlow);

	if(alpha <= zVal[eId] + tol){
		for(int v = 0; v < nVertex; v++)
			if(!cut_1[v])
				unite(edges[eId].u, v);
	}

	return min(alpha, zVal[eId]);
}

bool Problem::buildContFeasibilityCut(int i, vector<double>& zVal){

	if(algorithm == 4){
		shuffle(destination.begin(), destination.end(), rng);
		for(int t : destination){
			double maxFlow = myGoldberg(t, zVal, i);
			if(maxFlow < 1 - tol){
				for(auto e : edges){
					if(!scenarioUnsafeEdge[i].count(e.id)){
						if(cut[e.u] && !cut[e.v] || cut[e.v] && !cut[e.u])
							expr += getZ(e.id);
					}
				}
				expr -= 1;
				return true;
			}
		}
	}
	if(algorithm == 3){
		vector<double> xBar(nEdge, 0);
		for(int v = 0; v < nVertex; v++){
			parent[v] = v;
			rank[v] = 0;
		}
		double checkTot = 0.0;
		for(auto e : edges){
			if(zVal[e.id] > tol && !scenarioUnsafeEdge[i].count(e.id)){
				if(find(e.u) != find(e.v)){
					xBar[e.id] = findMaxPossibleValue(e.id, xBar, zVal, i);
					checkTot += xBar[e.id];
				}
			}
		}
		double check = 0;
		for(auto e : edges){
			if(find(e.u) != find(e.v) && !scenarioUnsafeEdge[i].count(e.id)){
				expr += getZ(e.id);
				check += zVal[e.id];
			}
		}
		unordered_set<int> parentSet(parent.begin(), parent.end());
		int nComponent = (int) parentSet.size(); // parents.size();
		expr -= (nComponent - 1);
		if(check < nComponent - 1 - tol)
			return true;
	}
	return false;
}