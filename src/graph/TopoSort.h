#pragma once
#include <vector>
#include "CourseGraph.h"
struct TopoResult {
    bool success = false;
    std::vector<int> order;
};
TopoResult topoSort(const CourseGraph& topo); //Kahn