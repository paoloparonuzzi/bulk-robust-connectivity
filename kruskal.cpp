//
// Created by Paolo on 28/04/2022.
//

#include "Problem.h"

using namespace std;

int Problem::find(int i){
	if (parent[i] == i)
		return i;
	return parent[i] = find(parent[i]);
}

void Problem::unite(int x, int y)
{
	int s1 = find(x);
	int s2 = find(y);
	if (s1 != s2) {
		if (rank[s1] < rank[s2])
			parent[s1] = s2;
		else if (rank[s1] > rank[s2])
			parent[s2] = s1;
		else {
			parent[s2] = s1;
			rank[s1] += 1;
		}
	}
}

int Problem::runKruskal(){

	for (int i = 0; i < nVertex; i++) {
		parent[i] = i;
		rank[i] = 0;
	}
	vector<vector<int>> edgeList;
	for(int e = 0; e < nEdge; e++)
		edgeList.push_back({(int) edges[e].cost, edges[e].v, edges[e].u});
	sort(edgeList.begin(), edgeList.end());
	int mstSol = 0;
	for (auto edge : edgeList) {
		int w = edge[0];
		int x = edge[1];
		int y = edge[2];

		// Take this edge in MST if it does
		// not forms a cycle
		if (find(x) != find(y)) {
			unite(x, y);
			mstSol += w;
			scenarioUnsafeEdge.push_back({edgeIndex(x, y)});
		}
	}

	cout << "mstSol: " << mstSol << endl;

	return mstSol;
}