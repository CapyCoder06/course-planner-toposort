#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

struct ClusterResult {
    unordered_map<string, int> courseToCluster; 
    vector<int> clusterCredits;                      
    bool feasible = true;
    string note;
};

class Clusterizer {
public:
    ClusterResult buildClusters(
        const unordered_map<string, int>& courseCredits,
        const unordered_map<string, vector<string>>& coreqs,
        int maxQuota
    );

private:
    void dfsCluster(
        const string& course,
        int clusterId,
        const unordered_map<string, vector<string>>& coreqs,
        unordered_map<string, int>& visited,
        unordered_map<string, int>& courseToCluster,
        int& totalCredits,
        const unordered_map<string, int>& courseCredits
    );
};
