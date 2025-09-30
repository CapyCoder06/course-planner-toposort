#include "CourseGraph.h"
#include <stdexcept>
#include <string>

void CourseGraph::build(const Curriculum& cur) {
    // 1) Map id -> idx & idx -> id
    idToIdx.clear();
    idxToId.clear();

    cur.for_each([&](const Course& c) {
        if (c.id.empty()) {
            throw std::runtime_error("CourseGraph: empty id");
        }
        if (idToIdx.count(c.id)) {
            throw std::runtime_error(std::string("CourseGraph: duplicate id: ") + c.id);
        }
        int idx = static_cast<int>(idToIdx.size());
        idToIdx.emplace(c.id, idx);
        idxToId.push_back(c.id);
    }); 

    // 2) Khởi tạo khung đồ thị
    V = static_cast<int>(idToIdx.size());
    adj.assign(V, {});   // V danh sách kề rỗng
    indeg.assign(V, 0);  // V số 0

    // 3) Thêm cạnh prereq -> course, tính indeg
    cur.for_each([&](const Course& c) {
        const int u = idToIdx.at(c.id); // course (đích)
        for (const auto& preId : c.prerequisite) {
            auto it = idToIdx.find(preId);
            if (it == idToIdx.end()) {
                throw std::runtime_error(
                    std::string("CourseGraph: unknown prerequisite '") + preId +
                    "' required by '" + c.id + "'"
                );
            }
            const int v = it->second;   // prereq (nguồn)
            adj[v].push_back(u);        // v -> u
            indeg[u]++;
        }
    });
}
