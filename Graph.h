// Graph.h
#pragma once
#include <vector>
#include <string>
using namespace std;

class Graph {
public:
	Graph() {}
	~Graph() {}

	void addEdge(string st1, string st2);
	void addVertex(int vnumber);

	// Ссылки на матрицу расстояний
	void print_friends(const vector<vector<int>>& matrix_v, string name_user);
	void print_3h(const vector<vector<int>>& matrix_v, string name_user);
	void print_other(const vector<vector<int>>& matrix_v, string name_user);

	void findMinDistancesFloyd(string name_user);
	bool edgeExists(int v1, int v2);

	vector<vector<int>> matrix;
	int size_matrix = 0;
	vector<string> vname;
	vector<int> vertexes;
};