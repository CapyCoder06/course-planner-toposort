#pragma once
#include <string>
#include <vector>
#include "../planner/TermAssigner.h"
#include "../graph/CourseGraph.h"
#include "../model/Curriculum.h"
#include "../model/PlanConstraints.h"

namespace planner
{

    struct EnhancedPlanResult
    {
        bool feasible;
        int totalTermsUsed;
        int totalCredits;

        struct TermInfo
        {
            int termNumber;
            std::vector<std::string> courseIds;
            std::vector<std::string> courseNames;
            std::vector<int> courseCredits;
            int totalCredits;
        };

        std::vector<TermInfo> terms;
        std::vector<std::string> notes;
        std::vector<std::string> warnings;

        std::string generatedAt;
        PlanConstraints constraints;
    };

    class Writer
    {
    public:
        static EnhancedPlanResult enhance(
            const PlanResult &planResult,
            const CourseGraph &graph,
            const Curriculum &curriculum,
            const PlanConstraints &constraints);

        static void writeJson(
            const EnhancedPlanResult &result,
            const std::string &filepath);

        static void writeMarkdown(
            const EnhancedPlanResult &result,
            const std::string &filepath);

        static std::string toJsonString(const EnhancedPlanResult &result);

        static std::string toMarkdownString(const EnhancedPlanResult &result);

        static EnhancedPlanResult loadFromJson(const std::string &filepath);

    private:
        static std::string getCurrentTimestamp();
        static std::string escapeMarkdown(const std::string &text);
    };

}