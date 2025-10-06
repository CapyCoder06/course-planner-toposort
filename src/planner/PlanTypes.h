#pragma once
#include <string>
#include <vector>

using namespace std;

struct Term {
    int index;
    vector<string> courseIds;
    int credits;
};

struct PlanResult {
    bool feasible;
    vector<Term> terms;
    string notes;
};
