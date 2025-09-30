#include "CycleDiagnosis.h"
#include <algorithm>
using namespace std;
static bool dfs(int u, const CourseGraph& g, vector<int>& color, vector<int>& parent, vector<int>& outCycle) {
    color[u] = 1;
    for (int v : g.adj[u]) {
        if (color[v] == 0) {
            parent[v] = u;
            if (dfs(v, g, color, parent, outCycle)) {
                return true;
            }
        }else if (color[v] == 1) {
            outCycle.clear();
            int x = u;
            outCycle.push_back(v);
            while (x != v && x != -1) {
                outCycle.push_back(x);
                x = parent[x];
            }
            if (x == v) {
                reverse(outCycle.begin(), outCycle.end());
            }else {
                outCycle.clear();
            }
            return true;
        }
    }
    color[u] = 2;
    return false;
}
vector<int> findOneCycle(const CourseGraph& g) {
    const int V = g.V;
    vector<int> color(V, 0), parent(V, -1), cycle;
    for (int u = 0; u < V; u++) {
        if (color[u] == 0) {
            if (dfs(u, g, color, parent, cycle)) return cycle;
        }
    }
    return {};
}