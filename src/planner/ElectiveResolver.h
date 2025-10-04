#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

struct ElectiveGroup {
    string groupId;                         
    vector<string> courseIds;               
    int requiredCount;                     
    unordered_map<string, int> coursePriority;
};

struct ElectiveResult {
    unordered_set<string> selectedCourses; 
    bool feasible = true;
    string message;                        
};

class ElectiveResolver {
public:
    ElectiveResult resolve(
        const vector<ElectiveGroup>& electiveGroups,  
        const unordered_map<string, int>& courseCredits,
        const unordered_map<string, vector<string>>& prerequisites 
    );

private:
    bool validatePrerequisites(
        const unordered_set<string>& selectedCourses, 
        const unordered_map<string, vector<string>>& prerequisites
    );
};