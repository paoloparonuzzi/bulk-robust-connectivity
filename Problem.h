//
// Created by Paolo on 26/04/2022.
//

#ifndef BULKROBUSTMATCH_PROBLEM_H
#define BULKROBUSTMATCH_PROBLEM_H

#include <gurobi_c++.h>
#include <vector>
#include <unordered_set>
#include <fstream>
#include <map>
#include <bits/stdc++.h>
#include <random>
#include <lemon/list_graph.h>
#include "Instance.h"

using namespace lemon;

struct Edge{
    Edge(int idd, int uu, int ww, double ccost);
	int id;
    int u;
    int v;
    double cost;
};

struct Problem{

    // Problem data and constructor
    int nVertex;
    int nEdge;
    int nScenario;
	int algorithm;
	int nEdgeScenario;
	int instSeed;
	double tol;
	std::default_random_engine rng;

    std::vector<Edge> edges;
    std::map<std::pair<int, int>, int> edgeMap;
    int edgeIndex(int u, int v);
    void printEdge(int e);
	std::vector<std::unordered_set<int>> scenarioUnsafeEdge;
	std::vector<std::unordered_set<int>> adj;
	std::vector<int> orderedScenario;
	std::vector<int> destination;

    GRBEnv env;

    explicit Problem(Instance& instance, int v_algorithm, int v_nMaxEdge);
    void buildBulkInstance();
	void writeInstanceFile();
	double myGoldberg(int t, const std::vector<double>& zVal, int i);

    // functions to solve MST
    std::vector<int> parent;
	std::vector<int> rank;
	int find(int i);
	void unite(int x, int y);
	int runKruskal();

	// functions to build and solve compact model
	GRBModel flowModel = GRBModel(env);
	GRBVar getZc(int e);
	GRBVar getFlowF(int u, int v, int t, int i);
	void buildFlowModel(double timeLimit, int lp);
	void solveFlowModel();
	// void printCompactModelSolution();
	void runFlowModel(double timeLimit, int lp);

    // functions to build and solve decomposition method
    int nAddedCut;
	double callbackTime;
	GRBModel masterModel = GRBModel(env);
	std::vector<std::vector<GRBModel>> modelFlowSub;
	GRBVar getZ(int e);
	GRBVar getSubF(int i, int t, int u, int v);
	GRBVar getSubFlowZ(int i, int t, int eId);
	GRBLinExpr expr;
	std::vector<GRBLinExpr> exprVec;
	void buildFlowSubProblem(int i, int t);
	bool gurobiFeasibilityCut(int i, std::vector<double>& zVal);
	double myGoldberdAux();
	double findMaxPossibleValue(int eId, std::vector<double>& xBar, std::vector<double>& zVal, int i);
	bool buildContFeasibilityCut(int i, std::vector<double>& zVal);
	void dfs(int v, int i, std::vector<double>& zVal, int countComp);
	bool buildIntFeasibilityCut(int i, std::vector<double>& zVal);
    void runBenders(double timeLimit);

	std::vector<int> visited;
	std::unordered_set<int> component;
	std::vector<std::unordered_set<int>> components;

	// functions to build and solve linear relaxations
	void linearRelaxation(double timeLimit);
	GRBModel lpModel = GRBModel(env);
	GRBVar getZlp(int e);

    // output
    std::ofstream output;
    std::string outputLine;

	// LEMON
	ListDigraph lemon_graph;
	std::vector<ListDigraph::Node> lemon_nodes;
	std::vector<ListDigraph::Arc> lemon_arcs;
	ListDigraph::ArcMap<double> lemon_capacity;
	std::vector<bool> cut;

	ListDigraph lemon_graph_1;
	std::vector<ListDigraph::Node> lemon_nodes_1;
	std::vector<ListDigraph::Arc> lemon_arcs_1;
	ListDigraph::ArcMap<double> lemon_capacity_1;
	std::vector<bool> cut_1;

};

#endif //BULKROBUSTMATCH_PROBLEM_H