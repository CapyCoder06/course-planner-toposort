#include "Explain.h"
#include <algorithm>

using namespace std;

Explain::Explain(const unordered_map<string, vector<string>>& prereq)
    : prereqTable(prereq) {}

vector<string> Explain::dfsLongestPath(
    const string& course,
    unordered_map<string, vector<string>>& memo,
    unordered_set<string>& visiting
) const {

    if (memo.count(course)) return memo[course];

    if (visiting.count(course)) return {course};

    visiting.insert(course);
    vector<string> longest;

    auto found = prereqTable.find(course);
    if (found != prereqTable.end()) {
        for (auto& pre : found->second) {
            auto chain = dfsLongestPath(pre, memo, visiting);
            if (chain.size() > longest.size())
                longest = chain;
        }
    }

    visiting.erase(course);
    longest.push_back(course);
    memo[course] = longest;
    return longest;
}

vector<string> Explain::whyPlaced(const string& courseId) const {
    unordered_map<string, vector<string>> memo;
    unordered_set<string> visiting;

    if (!prereqTable.count(courseId))
        return {courseId};

    return dfsLongestPath(courseId, memo, visiting);
}
