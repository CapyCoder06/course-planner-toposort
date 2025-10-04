#pragma once
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

struct HintNote {
    string message;                              
    unordered_map<string, string> actions;         
};

class Hints {
public:
    static vector<HintNote> analyze(
        int numTerms,
        int maxCredits,
        bool enforceCoreqTogether,
        bool electiveConflict,
        bool preferLightLoad
    );
};
