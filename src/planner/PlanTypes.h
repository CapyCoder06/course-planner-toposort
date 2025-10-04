#pragma once
#include <string>
#include <vector>

using namespace std;

struct Term {
    int index;            
    vector<std::string> courseIds;
    int credits = 0;
};

struct PlanResult {
    bool feasible = true;
    vector<Term> terms;
    string notes;
};
