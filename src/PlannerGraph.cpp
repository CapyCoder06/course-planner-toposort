#include "PlannerService.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <queue>
#include <algorithm>
#include <functional>
#include <unordered_set>

using nlohmann::json;

// ---------------- small utils ----------------
static std::string up(std::string v) {
    for (auto &ch : v) ch = (char)toupper((unsigned char)ch);
    return v;
}

// ---------------- Graph building ----------------
bool PlannerService::buildGraph(std::string& err) {
    adj_.clear();
    indeg_.clear();

    for (auto& [id, c] : courses_) {
        if (!indeg_.count(id)) indeg_[id] = 0;
        for (auto& pre : c.prereqs) {
            if (!courses_.count(pre)) continue;
            adj_[pre].push_back(id);
            indeg_[id]++;
            if (!indeg_.count(pre)) indeg_[pre] = 0;
        }
    }
    for (auto& [id, _] : courses_) if (!adj_.count(id)) adj_[id] = {};
    return true;
}

// ---------------- Topological sort ----------------
bool PlannerService::topoSort(std::vector<std::string>& order, std::string& err) const {
    order.clear();
    auto deg = indeg_;
    std::queue<std::string> q;
    for (auto& [v, d] : deg) if (d == 0) q.push(v);

    while (!q.empty()) {
        auto u = q.front(); q.pop();
        order.push_back(u);
        for (auto& v : adj_.at(u))
            if (--deg[v] == 0) q.push(v);
    }

    if (order.size() != courses_.size()) {
        err = "Curriculum has cycles.";
        return false;
    }
    return true;
}

// ---------------- Cycle detection ----------------
bool PlannerService::findCycle(std::vector<std::string>& cyc) const {
    cyc.clear();
    enum { W = 0, G = 1, B = 2 };
    std::unordered_map<std::string, int> col;
    std::unordered_map<std::string, std::string> par;

    for (auto& [id, _] : courses_) { col[id] = W; par[id] = ""; }

    std::function<bool(const std::string&)> dfs = [&](const std::string& u)->bool {
        col[u] = G;
        auto it = adj_.find(u);
        if (it != adj_.end()) {
            for (auto& v : it->second) {
                if (!courses_.count(v)) continue;
                if (col[v] == W) { par[v] = u; if (dfs(v)) return true; }
                else if (col[v] == G) {
                    cyc = {v, u, v};
                    return true;
                }
            }
        }
        col[u] = B;
        return false;
    };

    for (auto& [id, _] : courses_)
        if (col[id] == W && dfs(id)) return true;
    return false;
}

// ---------------- JSON loader ----------------
bool PlannerService::loadCurriculum(const std::string& jsonPath, std::string& err) {
    err.clear();
    courses_.clear();
    std::ifstream f(jsonPath);
    if (!f) { err = "Cannot open file: " + jsonPath; return false; }

    json j;
    try { f >> j; }
    catch (const std::exception& ex) {
        err = std::string("JSON parse error: ") + ex.what();
        return false;
    }

    if (!j.contains("courses") || !j["courses"].is_array()) {
        err = "JSON must contain array 'courses'.";
        return false;
    }

    try {
        for (auto& x : j["courses"]) {
            Course c;
            c.id = x.at("id").get<std::string>();
            c.name = x.value("name", c.id);
            c.credits = x.value("credits", 0);
            c.track = x.value("track", std::string{});
            if (x.contains("prereq") && x["prereq"].is_array()) {
                for (auto& p : x["prereq"]) c.prereqs.push_back(p.get<std::string>());
            }
            courses_[c.id] = std::move(c);
        }
    } catch (const std::exception& ex) {
        err = std::string("Invalid course entry: ") + ex.what();
        return false;
    }

    return buildGraph(err);
}

std::vector<std::string> PlannerService::prereqsOf(const std::string& id) const {
    std::vector<std::string> out;
    auto it = courses_.find(id);
    if (it == courses_.end()) return out;
    for (auto& p : it->second.prereqs) if (courses_.count(p)) out.push_back(p);
    return out;
}
