#include "TopoSort.h"
#include <queue>
using namespace std;
TopoResult topoSort(const CourseGraph& topo) {
    TopoResult res;
    const int V = topo.V;
    vector<int> indeg = topo.indeg;
    queue<int> q;
    for (int u = 0; u < V; u++) {
        if (indeg[u] == 0) {
            q.push(u);
        }
    }
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        res.order.push_back(u);
        for (int v : topo.adj[u]) {
            if (--indeg[v] == 0) {
                q.push(v);
            }
        }
    }
    res.success = (int)res.order.size() == V;
    return res;
}