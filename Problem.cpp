//
// Created by Paolo on 26/04/2022.
//

#include "Problem.h"

#include <lemon/preflow.h>

using namespace std;

Problem::Problem(Instance& instance, int v_algorithm, int v_nMaxEdge) : lemon_capacity(lemon_graph), lemon_capacity_1(lemon_graph_1) {

    nVertex = instance.i_nVertex;
    nEdge = instance.i_nEdge;
	nAddedCut = 0;
	callbackTime = 0;
	algorithm = v_algorithm;
	nEdgeScenario = v_nMaxEdge;
	nScenario = 0;
	instSeed = instance.instSeed;
	tol = 0.001;

	parent.resize(nVertex);
	rank.resize(nVertex);
	for (int i = 0; i < nVertex; i++) {
		parent[i] = i;
		rank[i] = 0;
	}
    for(int u=0; u<nVertex; u++){
        for(int w=0; w<nVertex; w++)
            edgeMap[make_pair(u, w)] = -1;
    }

	adj.resize(nVertex);
    for(int e=0; e<nEdge; e++){
        edges.emplace_back(e, instance.u[e], instance.w[e], instance.i_cost[e]);
	    edgeMap[make_pair(instance.u[e], instance.w[e])] = e;
	    adj[edges[e].u].insert(edges[e].v);
	    adj[edges[e].v].insert(edges[e].u);
    }

	cut.resize(nVertex, false);
	for (int u = 0; u < nVertex; u++)
		lemon_nodes.push_back(lemon_graph.addNode());
	for(int e = 0; e < nEdge; e++){
		auto arc_1 = lemon_graph.addArc(lemon_nodes[edges[e].u], lemon_nodes[edges[e].v]);
		lemon_arcs.push_back(arc_1);
		lemon_capacity[arc_1] = 0;
	}
	for(int e = 0; e < nEdge; e++){
		auto arc_2 = lemon_graph.addArc(lemon_nodes[edges[e].v], lemon_nodes[edges[e].u]);
		lemon_arcs.push_back(arc_2);
		lemon_capacity[arc_2] = 0;
	}

	cut_1.resize(nVertex + 2, false);
	for (int u = 0; u < nVertex + 2; u++)
		lemon_nodes_1.push_back(lemon_graph_1.addNode());
	for(int e = 0; e < nEdge; e++){
		auto arc_1 = lemon_graph_1.addArc(lemon_nodes_1[edges[e].u], lemon_nodes_1[edges[e].v]);
		lemon_arcs_1.push_back(arc_1);
		lemon_capacity_1[arc_1] = 0;
	}
	for(int e = 0; e < nEdge; e++){
		auto arc_2 = lemon_graph_1.addArc(lemon_nodes_1[edges[e].v], lemon_nodes_1[edges[e].u]);
		lemon_arcs_1.push_back(arc_2);
		lemon_capacity_1[arc_2] = 0;
	}
	for(int u = 0; u < nVertex; u++){
		auto arc = lemon_graph_1.addArc(lemon_nodes_1[nVertex], lemon_nodes_1[u]);
		lemon_arcs_1.push_back(arc);
		lemon_capacity_1[arc] = 0;
	}
	for(int u = 0; u < nVertex; u++){
		auto arc = lemon_graph_1.addArc(lemon_nodes_1[u], lemon_nodes_1[nVertex + 1]);
		lemon_arcs_1.push_back(arc);
		lemon_capacity_1[arc] = 0;
	}

	visited.resize(nVertex, 0);
	for(int t=1; t<nVertex; t++)
		destination.push_back(t);
	rng = default_random_engine{};
}

double Problem::myGoldberg(int t, const std::vector<double>& zVal, int i){

	for(int e = 0; e < nEdge; e++){
		lemon_capacity[lemon_arcs[e]] = zVal[e];
		lemon_capacity[lemon_arcs[e + nEdge]] = zVal[e];
	}
	// forbid scenario edges
	for(auto e : scenarioUnsafeEdge[i]){
		lemon_capacity[lemon_arcs[e]] = 0;
		lemon_capacity[lemon_arcs[e + nEdge]] = 0;
	}

	Preflow<ListDigraph, ListDigraph::ArcMap<double>> preflow(lemon_graph, lemon_capacity, lemon_nodes[0], lemon_nodes[t]);
	preflow.run();

	for (int u = 0; u < nVertex; ++u)
		cut[u] = preflow.minCut(lemon_nodes[u]);

	return preflow.flowValue();
}

double Problem::myGoldberdAux(){
	Preflow<ListDigraph, ListDigraph::ArcMap<double>> preflow(lemon_graph_1, lemon_capacity_1, lemon_nodes_1[nVertex], lemon_nodes_1[nVertex+1]);
	preflow.run();
	for (int u = 0; u < nVertex + 2; ++u)
		cut_1[u] = preflow.minCut(lemon_nodes_1[u]);
	return preflow.flowValue();
}


Edge::Edge(int idd, int uu, int ww, double ccost){
	id = idd;
    u = uu;
	v = ww;
    cost = ccost;
}

int Problem::edgeIndex(int u, int v){
	if(u < v)
		return edgeMap[make_pair(u, v)];
	else
		return edgeMap[make_pair(v, u)];
}

void Problem::printEdge(int e){
    cout << "(" << edges[e].u << ", " << edges[e].v << ") ";
}

bool sortbysec(const pair<int,int> &a, const pair<int,int> &b)
{
	return (a.second > b.second);
}

GRBVar Problem::getZc(int e){
	return flowModel.getVarByName("z_" + to_string(edges[e].u) + "_" + to_string(edges[e].v));
}

GRBVar Problem::getZ(int e){
	return masterModel.getVarByName("z_" + to_string(edges[e].u) + "_" + to_string(edges[e].v));
}

GRBVar Problem::getZlp(int e){
	return lpModel.getVarByName("z_" + to_string(edges[e].u) + "_" + to_string(edges[e].v));
}

GRBVar Problem::getSubF(int u, int v, int t, int i){
	return  modelFlowSub[i][t].getVarByName("fSub_" + to_string(u) + "_" + to_string(v));
}

GRBVar Problem::getSubFlowZ(int i, int t, int eId){
	return  modelFlowSub[i][t].getVarByName("zSub_" + to_string(eId));
}

void Problem::buildBulkInstance(){

	nScenario = (int) scenarioUnsafeEdge.size();
	for(auto&scenario : scenarioUnsafeEdge){
		set<int> alreadyInserted;
		for(auto e : scenario)
			alreadyInserted.insert(e);
		int n = 1;
		while(n < nEdgeScenario){
			int randE = rand()%nEdge;
			if(!alreadyInserted.count(randE)){
				scenario.insert(randE);
				alreadyInserted.insert(randE);
				n++;
			}
		}
	}

	for(int i = 0; i <nScenario; i++)
		orderedScenario.push_back(i);
}

void Problem::writeInstanceFile(){

	ofstream instanceFile;
	instanceFile.open("fgcpInstances/FGCP_" + to_string(nVertex) + "_" + to_string(nScenario) + "_" +
	                  to_string(nEdgeScenario) + "_" + to_string(instSeed) + ".txt", ios_base::trunc);

	if(instanceFile.is_open()){
		instanceFile << nVertex << "\t" << nEdge << endl;
		for(auto e : edges)
			instanceFile << e.u << "\t" << e.v << "\t" << e.cost << endl;
		instanceFile << nScenario << endl;
		for(const auto& scenario : scenarioUnsafeEdge){
			for(auto e: scenario)
				instanceFile << e << "\t";
			instanceFile << endl;
		}
		instanceFile << endl;
	}
	instanceFile.close();

}