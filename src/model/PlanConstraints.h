#pragma once
#include <vector>
#include <stdexcept>
using namespace std;

struct PlanConstraints 
{
    int numTerms;
    int maxCreditsPerTerm;
    int minCreditsPerTerm;
    bool enforceCoreqTogether;
    vector<int> offered_terms;

    void validate() const;
};