#pragma once
#include <vector>
#include <string>
#include <utility>
#include "../graph/CourseGraph.h"
#include "../graph/TopoSort.h"
#include "../model/PlanConstraints.h"

struct PlanResult {
    bool ok = true;                       // false if infeasible (e.g., quota or numTerms exhausted)
    std::vector<int> termOfIdx;           // assignment per course idx, 1-based term; 0 means unassigned
    std::vector<std::string> notes;       // hints or reasons when infeasible or adjusted
};

// Greedy heuristic: iterate in topo order, place each course at max(earliestTerm, currentTerm).
// If quota exceeded, advance to next term until fits. If > numTerms -> infeasible.
PlanResult assignTermsGreedy(const CourseGraph& g,
                             const TopoResult& topo,
                             const std::vector<int>& earliestTermByIdx, // size V, >=1
                             const std::vector<int>& creditsByIdx,      // size V, >=0
                             const PlanConstraints& constraints);
