/*
 * CourseGraph
 * - V: số đỉnh
 * - adj[v]: danh sách u sao cho v -> u (prereq -> course)
 * - indeg[u]: số cạnh vào u
 * - idToIdx: id -> index
 * - idxToId: index -> id (debug/in kết quả)
 *
 * AC: Interface đủ cho topo sort, longest path, cycle detection.
 */

#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include "Curriculum.h"
struct CourseGraph {
    int V = 0;
    std::vector<std::vector<int>> adj;
    std::vector<int> indeg;
    std::unordered_map<std::string, int> idToIdx;
    std::vector<std::string> idxToId;

    void build(const Curriculum& cur);
};