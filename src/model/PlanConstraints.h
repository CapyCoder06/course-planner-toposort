#pragma once
#include <vector>
#include <stdexcept>
using namespace std;

struct PlanConstraints 
{
    int numTerms;
    int maxcredits;
    int mincredits;
    bool enforceCoreqTogether;
    vector<int> offered_terms;

    void validate() const;
};