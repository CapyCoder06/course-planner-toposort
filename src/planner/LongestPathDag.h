#pragma once
#include <vector>
#include <stdexcept>
#include "../graph/CourseGraph.h"
#include "../graph/TopoSort.h"
struct EarliestTerms {
    bool ok = true;
    std::vector<int> termByIdx;
};
EarliestTerms computeEarliestTerms(const CourseGraph& g, const TopoResult& topo);