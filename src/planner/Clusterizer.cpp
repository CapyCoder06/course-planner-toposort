#include "Clusterizer.h"
#include <stack>

using namespace std;

// Duyệt các môn trong cùng cụm coreq bằng DFS (dùng stack để tránh đệ quy)
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

        if (visited[cur]) continue; // đã thăm rồi thì bỏ qua

        visited[cur] = 1;
        courseToCluster[cur] = clusterId;

        // cộng dồn số tín chỉ của môn hiện tại
        auto itCredit = courseCredits.find(cur);
        if (itCredit != courseCredits.end()) {
            totalCredits += itCredit->second;
        }

        // thêm các môn coreq liên kết để duyệt tiếp
        auto it = coreqs.find(cur);
        if (it != coreqs.end()) {
            for (const string& nxt : it->second) {
                if (!visited[nxt]) st.push(nxt);
            }
        }
    }
}

// Gom các môn thành cụm (cluster) dựa trên corequisite
ClusterResult Clusterizer::buildClusters(
    const unordered_map<string, int>& courseCredits,
    const unordered_map<string, vector<string>>& coreqs,
    int maxQuota
) {
    ClusterResult result;
    result.feasible = true;
    unordered_map<string, int> visited;
    int clusterId = 0;

    // Duyệt qua từng môn để tạo cụm mới nếu chưa được xử lý
    for (const auto& kv : courseCredits) {
        const string& course = kv.first;
        if (!visited[course]) {
            int totalCredits = 0;

            // gom các môn liên quan (coreq) vào cùng một cụm
            dfsCluster(course, clusterId, coreqs, visited,
                       result.courseToCluster, totalCredits, courseCredits);

            result.clusterCredits.push_back(totalCredits);

            // nếu tổng tín chỉ vượt giới hạn quota, đánh dấu là không khả thi
            if (totalCredits > maxQuota) {
                result.feasible = false;
                result.note += "Cluster " + to_string(clusterId) +
                               " vượt quota (" + to_string(totalCredits) +
                               " > " + to_string(maxQuota) + "). ";
            }

            clusterId++;
        }
    }

    return result;
}
