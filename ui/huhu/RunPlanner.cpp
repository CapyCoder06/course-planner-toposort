#include "RunPlanner.h"
#include <fstream>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <string>

// JSON single-header (external/nlohmann/json.hpp)
#include <nlohmann/json.hpp>
using nlohmann::json;

// ==== CORE ====
#include "graph/CourseGraph.h"   // V, adj, indeg, idToIdx, idxToId  (Kahn inputs)  // :contentReference[oaicite:2]{index=2}
#include "graph/TopoSort.h"      // TopoResult { success, order }, topoSort(...)     // :contentReference[oaicite:3]{index=3}

std::vector<std::string> topoFromJsonFile(const std::string& path)
{
    // 1) Parse JSON
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Cannot open file: " + path);

    json J; in >> J;
    if (!J.contains("courses") || !J["courses"].is_array())
        throw std::runtime_error("JSON missing 'courses' array");

    // 2) Gán chỉ số liên tục cho từng id
    CourseGraph g;
    int next = 0;
    for (const auto& c : J["courses"]) {
        const std::string id = c.at("id").get<std::string>();
        if (!g.idToIdx.count(id)) {
            g.idToIdx[id] = next++;
            g.idxToId.push_back(id);
        }
    }
    g.V = static_cast<int>(g.idxToId.size());
    g.adj.assign(g.V, {});
    g.indeg.assign(g.V, 0);

    // 3) Tạo cạnh prereq -> course và tính indegree
    for (const auto& c : J["courses"]) {
        const std::string id = c.at("id").get<std::string>();
        int u = g.idToIdx.at(id); // course index
        if (c.contains("prereq")) {
            for (const auto& pre : c["prereq"]) {
                const std::string p = pre.get<std::string>();
                // chỉ thêm cạnh nếu prereq có trong danh sách môn
                auto it = g.idToIdx.find(p);
                if (it != g.idToIdx.end()) {
                    int preIdx = it->second;
                    g.adj[preIdx].push_back(u); // prereq -> course
                    g.indeg[u]++;               // course có thêm 1 incoming
                }
            }
        }
    }

    // 4) Topo sort bằng core
    TopoResult r = topoSort(g);   // trả indices theo Kahn
    std::vector<std::string> out;
    out.reserve(r.order.size());
    for (int ix : r.order) {
        // map index -> id để hiển thị
        out.push_back(g.idxToId.at(ix));
    }
    return out;
}
