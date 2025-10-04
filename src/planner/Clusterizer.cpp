#include "Clusterizer.h"
#include <stack>

using namespace std;

void Clusterizer::dfsCluster(
    const string& course,
    int clusterId,
    const unordered_map<string, vector<string>>& coreqs,
    unordered_map<string, int>& visited,
    unordered_map<string, int>& courseToCluster,
    int& totalCredits,
    const unordered_map<string, int>& courseCredits
) {
    stack<string> st;
    st.push(course);
    while (!st.empty()) {
        string cur = st.top();
        st.pop();
        if (visited[cur]) continue;
        visited[cur] = 1;
        courseToCluster[cur] = clusterId;
        auto itCredit = courseCredits.find(cur);
        if (itCredit != courseCredits.end()) {
            totalCredits += itCredit->second;
        }
        auto it = coreqs.find(cur);
        if (it != coreqs.end()) {
            for (const string& nxt : it->second) {
                if (!visited[nxt]) st.push(nxt);
            }
        }
    }
}

ClusterResult Clusterizer::buildClusters(
    const unordered_map<string, int>& courseCredits,
    const unordered_map<string, vector<string>>& coreqs,
    int maxQuota
) {
    ClusterResult result;
    unordered_map<string, int> visited;
    int clusterId = 0;

    for (const auto& kv : courseCredits) {
        const string& course = kv.first;
        if (!visited[course]) {
            int totalCredits = 0;
            dfsCluster(course, clusterId, coreqs, visited,
                       result.courseToCluster, totalCredits, courseCredits);
            result.clusterCredits.push_back(totalCredits);
            if (totalCredits > maxQuota) {
                result.feasible = false;
                result.note += "Cluster " + to_string(clusterId) +
                               " vượt quota (" + to_string(totalCredits) + " > " +
                               to_string(maxQuota) + "). ";
            }
            clusterId++;
        }
    }

    return result;
}
