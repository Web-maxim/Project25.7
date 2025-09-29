// Graph.cpp
#include "Graph.h"
#include <iostream>
#include <vector>
#include <string>
using namespace std;

#define VERYBIGINT 1000000000

void Graph::addEdge(string st1, string st2) {
    int v1 = -1;
    int v2 = -1;
    for (int i = 0; i < (int)vname.size(); i++) {
        if (vname[i] == st1) v1 = i;
        if (vname[i] == st2) v2 = i;
    }
    if (v1 == -1) {
        vname.push_back(st1);
        v1 = (int)vname.size() - 1;
        addVertex(v1);
    }
    if (v2 == -1) {
        vname.push_back(st2);
        v2 = (int)vname.size() - 1;
        addVertex(v2);
    }
    matrix[v1][v2] = 1;
    matrix[v2][v1] = 1;
}

void Graph::addVertex(int value) {
    vertexes.push_back(value);
    size_matrix++;
    vector<vector<int>> newMatrix(size_matrix, vector<int>(size_matrix, 0));
    for (int i = 0; i < size_matrix - 1; i++) {
        for (int j = 0; j < size_matrix - 1; j++) {
            newMatrix[i][j] = matrix[i][j];
        }
    }
    matrix = newMatrix;
}

bool Graph::edgeExists(int v1, int v2) {
    return matrix[v1][v2] > 0;
}

void Graph::findMinDistancesFloyd(string name_user) {
    // Используем вектор для защиты от утечек.
    vector<vector<int>> weights(size_matrix, vector<int>(size_matrix, VERYBIGINT));

    for (int i = 0; i < size_matrix; i++) {
        for (int j = 0; j < size_matrix; j++) {
            if (i == j) weights[i][j] = 0;
            else if (edgeExists(i, j)) weights[i][j] = matrix[i][j];
        }
    }

    for (int k = 0; k < size_matrix; k++) {
        for (int i = 0; i < size_matrix; i++) {
            for (int j = 0; j < size_matrix; j++) {
                // Защита от переполнения «INF + X»
                if (weights[i][k] < VERYBIGINT && weights[k][j] < VERYBIGINT) {
                    int cand = weights[i][k] + weights[k][j];
                    if (cand < weights[i][j]) weights[i][j] = cand;
                }
            }
        }
    }

    print_friends(weights, name_user);
    print_3h(weights, name_user);
    print_other(weights, name_user);


}

void Graph::print_friends(const vector<vector<int>>& matrix_v, string name_user) {
    for (int i = 0; i < size_matrix; i++) {
        if (vname[i] == name_user && size_matrix > 1) {
            cout << vname[i] << " friends: ";
            int f = 0;
            for (int j = 0; j < size_matrix; j++) {
                if (matrix_v[i][j] == 1) {
                    if (f > 0) cout << ", ";
                    cout << vname[j];
                    f++;
                }
            }
            if (f > 0) cout << "." << endl;
            else cout << " you don`t have friends. " << endl;
            break;
        }
    }
}

void Graph::print_3h(const vector<vector<int>>& matrix_v, string name_user) {
    for (int i = 0; i < size_matrix; i++) {
        if (vname[i] == name_user) {
            cout << vname[i] << " could know: ";
            int f = 0;
            for (int j = 0; j < size_matrix; j++) {
                if (matrix_v[i][j] < 4 && matrix_v[i][j] >= 2) {
                    if (f > 0) cout << ", ";
                    cout << vname[j];
                    f++;
                }
            }
            if (f > 0) cout << "." << endl;
            else cout << " you need to add friends. " << endl;
            break;
        }
    }
}

void Graph::print_other(const vector<vector<int>>& matrix_v, string name_user) {
    // Упрощён и ускорен подсчёт «других» пользователей, без вложенных полных циклов и set
    cout << "Other online users: " << endl;

    int me = -1;
    for (int i = 0; i < size_matrix; ++i) if (vname[i] == name_user) { me = i; break; }
    if (me == -1) { cout << "(unknown user)" << endl << endl; return; }

    for (int j = 0; j < size_matrix; j++) {
        if (j == me) continue;
        if ((matrix_v[me][j] == 0 && me != j) || matrix_v[me][j] > 3) {
            cout << vname[j] << ", ";
        }
    }
    cout << endl << endl;
}