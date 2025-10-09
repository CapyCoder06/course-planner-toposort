#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

struct PlannedCourse {
    std::string id;
    std::string name;
    int credits{};
};

struct PlannedTerm {
    int index{}; // 1..8
    std::vector<PlannedCourse> courses;
    int totalCredits{}; // sum of credits
};

struct PlanResult {
    bool ok{false};
    std::string message;
    std::vector<PlannedTerm> terms; // size == 8 when ok
};

// 4 chuyên ngành
enum class Specialization { SE, NNS, IS, AI, NONE };

struct Course {
    std::string id;
    std::string name;
    int credits{};
    std::vector<std::string> prereqs; // from "prereq" in JSON
    std::string track;                // "General" | "ITCore" | "IT Core Elective" | "Spec:SE" | "SE" | "NNS" | "IS" | "AI" | ...
};

class PlannerService {
public:
    bool loadCurriculum(const std::string& jsonPath, std::string& err);
    PlanResult buildPlan(Specialization spec, int maxCreditsPerTerm = 28) const;
    

    // Để UI hiển thị cột Prerequisite
    std::vector<std::string> prereqsOf(const std::string& courseId) const;

private:
    // data
    std::unordered_map<std::string, Course> courses_;               // id -> Course
    std::unordered_map<std::string, std::vector<std::string>> adj_; // u -> [v,...]
    std::unordered_map<std::string, int> indeg_;                    // v -> indegree

    // tag/spec helpers
    static bool isGeneralTrack(std::string t);
    static bool isITCoreTrack(std::string t);
    static bool isITCoreElectiveTrack(std::string t);
    static std::string specKey(Specialization s);
    static bool isSpecCore(const Course& c, const std::string& skey);
    static bool isSpecElective(const Course& c, const std::string& skey);
    static bool isSpecProjectId(const std::string& id, const std::string& skey);
    static bool isCapstoneId(const std::string& id);
    

    // graph helpers
    bool buildGraph(std::string& err);
    bool topoSort(std::vector<std::string>& order, std::string& err) const;
    bool findCycle(std::vector<std::string>& cycleIds) const;
    
};
