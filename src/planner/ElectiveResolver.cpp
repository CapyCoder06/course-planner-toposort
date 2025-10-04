#include "ElectiveResolver.h"
#include <algorithm>
#include <sstream>

using namespace std;

ElectiveResult ElectiveResolver::resolve(
    const vector<ElectiveGroup>& groups,
    const unordered_map<string, int>& creditTable,
    const unordered_map<string, vector<string>>& prereqTable
) {
    ElectiveResult result;

    for (const ElectiveGroup& group : groups) {
        vector<string> pool = group.courseIds;

        sort(pool.begin(), pool.end(), [&](const string& a, const string& b) {
            int pa = 0, pb = 0;
            if (group.coursePriority.find(a) != group.coursePriority.end())
                pa = group.coursePriority.at(a);
            if (group.coursePriority.find(b) != group.coursePriority.end())
                pb = group.coursePriority.at(b);

            if (pa != pb) return pa > pb;

            int ca = 0, cb = 0;
            if (creditTable.find(a) != creditTable.end()) ca = creditTable.at(a);
            if (creditTable.find(b) != creditTable.end()) cb = creditTable.at(b);

            return ca < cb; 
        });

        if ((int)pool.size() < group.requiredCount) {
            result.feasible = false;

            ostringstream oss;
            oss << "Group " << group.groupId << " only has "
                << pool.size() << " courses, need " << group.requiredCount;
            result.message = oss.str();

            return result;
        }

        for (int i = 0; i < group.requiredCount; ++i) {
            result.selectedCourses.insert(pool[i]);
        }
    }

    if (!validatePrerequisites(result.selectedCourses, prereqTable)) {
        result.feasible = false;
        result.message = "Missing prerequisite(s) for chosen electives.";
    }

    return result;
}

bool ElectiveResolver::validatePrerequisites(
    const unordered_set<string>& chosen,
    const unordered_map<string, vector<string>>& prereqTable
) {
    for (const pair<const string, vector<string>>& entry : prereqTable) {
        const string& course = entry.first;
        const vector<string>& prereqs = entry.second;

        if (chosen.find(course) != chosen.end()) {
            for (const string& pre : prereqs) {
                if (chosen.find(pre) == chosen.end()) {
                    return false; 
                }
            }
        }
    }
    return true;
}
