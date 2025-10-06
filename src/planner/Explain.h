#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

using namespace std;

class Explain {
public:

    Explain(const unordered_map<string, vector<string>>& prereqTable);

    vector<string> whyPlaced(const string& courseId) const;

private:
    unordered_map<string, vector<string>> prereqTable;

    vector<string> dfsLongestPath(
        const string& course,
        unordered_map<string, vector<string>>& memo,
        unordered_set<string>& visiting
    ) const;
};
