#include "Writer.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <iomanip>
#include <string>

using json = nlohmann::json;
using namespace std;

namespace planner
{

    string Writer::getCurrentTimestamp()
    {
        auto now = chrono::system_clock::now();
        auto time_t_now = chrono::system_clock::to_time_t(now);
        stringstream ss;
        ss << put_time(localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    string Writer::escapeMarkdown(const string &text)
    {
        string result = text;
        return result;
    }

    EnhancedPlanResult Writer::enhance(
        const PlanResult &planResult,
        const CourseGraph &graph,
        const Curriculum &curriculum,
        const PlanConstraints &constraints)
    {
        EnhancedPlanResult enhanced;
        enhanced.feasible = planResult.ok;
        enhanced.constraints = constraints;
        enhanced.notes = planResult.notes;
        enhanced.generatedAt = getCurrentTimestamp();

        map<int, vector<int>> termCourseIndices;
        for (size_t idx = 0; idx < planResult.termOfIdx.size(); ++idx)
        {
            int term = planResult.termOfIdx[idx];
            if (term > 0)
            {
                termCourseIndices[term].push_back(idx);
            }
        }

        enhanced.totalTermsUsed = termCourseIndices.size();
        enhanced.totalCredits = 0;

        for (const auto &[termNum, indices] : termCourseIndices)
        {
            EnhancedPlanResult::TermInfo termInfo;
            termInfo.termNumber = termNum;
            termInfo.totalCredits = 0;

            for (int idx : indices)
            {
                string courseId = graph.idxToId.at(idx);
                const Course &course = curriculum.get(courseId);

                termInfo.courseIds.push_back(courseId);
                termInfo.courseNames.push_back(course.name);
                termInfo.courseCredits.push_back(course.credits);
                termInfo.totalCredits += course.credits;
            }

            enhanced.totalCredits += termInfo.totalCredits;
            enhanced.terms.push_back(termInfo);
        }

        sort(enhanced.terms.begin(), enhanced.terms.end(),
             [](const auto &a, const auto &b)
             { return a.termNumber < b.termNumber; });

        for (const auto &term : enhanced.terms)
        {
            if (term.totalCredits > constraints.maxCreditsPerTerm)
            {
                stringstream ss;
                ss << "Warning: Term " << term.termNumber
                   << " exceeds max credits (" << term.totalCredits
                   << " > " << constraints.maxCreditsPerTerm << ")";
                enhanced.warnings.push_back(ss.str());
            }
            if (term.totalCredits < constraints.minCreditsPerTerm)
            {
                stringstream ss;
                ss << "Warning: Term " << term.termNumber
                   << " below min credits (" << term.totalCredits
                   << " < " << constraints.minCreditsPerTerm << ")";
                enhanced.warnings.push_back(ss.str());
            }
        }

        return enhanced;
    }

    void Writer::writeJson(const EnhancedPlanResult &result, const string &filepath)
    {
        ofstream file(filepath);
        if (!file.is_open())
        {
            throw runtime_error("Cannot open file for writing: " + filepath);
        }

        file << toJsonString(result);
        file.close();
    }

    string Writer::toJsonString(const EnhancedPlanResult &result)
    {
        json j;

        j["metadata"] = {
            {"feasible", result.feasible},
            {"generatedAt", result.generatedAt},
            {"totalTermsUsed", result.totalTermsUsed},
            {"totalCredits", result.totalCredits}};

        j["constraints"] = {
            {"numTerms", result.constraints.numTerms},
            {"maxCreditsPerTerm", result.constraints.maxCreditsPerTerm},
            {"minCreditsPerTerm", result.constraints.minCreditsPerTerm},
            {"enforceCoreqTogether", result.constraints.enforceCoreqTogether}};

        json termsArray = json::array();
        for (const auto &term : result.terms)
        {
            json termObj;
            termObj["termNumber"] = term.termNumber;
            termObj["totalCredits"] = term.totalCredits;

            json coursesArray = json::array();
            for (size_t i = 0; i < term.courseIds.size(); ++i)
            {
                json courseObj;
                courseObj["id"] = term.courseIds[i];
                courseObj["name"] = term.courseNames[i];
                courseObj["credits"] = term.courseCredits[i];
                coursesArray.push_back(courseObj);
            }
            termObj["courses"] = coursesArray;

            termsArray.push_back(termObj);
        }
        j["terms"] = termsArray;

        if (!result.notes.empty())
        {
            j["notes"] = result.notes;
        }

        if (!result.warnings.empty())
        {
            j["warnings"] = result.warnings;
        }

        return j.dump(2);
    }

    void Writer::writeMarkdown(const EnhancedPlanResult &result, const string &filepath)
    {
        ofstream file(filepath);
        if (!file.is_open())
        {
            throw runtime_error("Cannot open file for writing: " + filepath);
        }

        file << toMarkdownString(result);
        file.close();
    }

    string Writer::toMarkdownString(const EnhancedPlanResult &result)
    {
        stringstream md;

        // Header
        md << "# Study Plan\n\n";

        // Metadata
        md << "## Summary\n\n";
        md << "- **Status**: " << (result.feasible ? "✅ Feasible" : "❌ Infeasible") << "\n";
        md << "- **Generated**: " << result.generatedAt << "\n";
        md << "- **Total Terms**: " << result.totalTermsUsed << " / " << result.constraints.numTerms << "\n";
        md << "- **Total Credits**: " << result.totalCredits << "\n";
        md << "- **Credits per Term**: " << result.constraints.minCreditsPerTerm
           << " - " << result.constraints.maxCreditsPerTerm << "\n\n";

        // Constraints
        md << "## Constraints\n\n";
        md << "| Parameter | Value |\n";
        md << "|-----------|-------|\n";
        md << "| Max Terms | " << result.constraints.numTerms << " |\n";
        md << "| Max Credits/Term | " << result.constraints.maxCreditsPerTerm << " |\n";
        md << "| Min Credits/Term | " << result.constraints.minCreditsPerTerm << " |\n";
        md << "| Enforce Coreq Together | " << (result.constraints.enforceCoreqTogether ? "Yes" : "No") << " |\n\n";

        // Terms
        if (!result.terms.empty())
        {
            md << "## Course Schedule\n\n";

            for (const auto &term : result.terms)
            {
                md << "### Term " << term.termNumber << " (" << term.totalCredits << " credits)\n\n";

                md << "| Course ID | Course Name | Credits |\n";
                md << "|-----------|-------------|--------:|\n";

                for (size_t i = 0; i < term.courseIds.size(); ++i)
                {
                    md << "| " << term.courseIds[i] << " | "
                       << escapeMarkdown(term.courseNames[i]) << " | "
                       << term.courseCredits[i] << " |\n";
                }

                md << "\n**Term Total**: " << term.totalCredits << " credits\n\n";
            }
        }

        // Warnings
        if (!result.warnings.empty())
        {
            md << "## ⚠️ Warnings\n\n";
            for (const auto &warning : result.warnings)
            {
                md << "- " << warning << "\n";
            }
            md << "\n";
        }

        // Notes
        if (!result.notes.empty())
        {
            md << "## 📝 Notes\n\n";
            for (size_t i = 0; i < result.notes.size(); ++i)
            {
                md << (i + 1) << ". " << result.notes[i] << "\n";
            }
            md << "\n";
        }

        // Credits breakdown chart (simple ASCII)
        if (!result.terms.empty()) {
            md << "## Credits Distribution\n\n";
            md << "```\n";

        constexpr int BAR_WIDTH = 40;
        static constexpr const char* BLOCK = "\xE2\x96\x88"; 

        int maxCredits = 0;
        for (const auto& term : result.terms) {
            maxCredits = std::max(maxCredits, term.totalCredits);
        }
        if (maxCredits <= 0) maxCredits = 1; 

        md << "[0]";
        md << std::string(BAR_WIDTH - 5, ' ') << "[" << maxCredits << "]\n";

        for (const auto& term : result.terms) {
            md << "Term " << std::setw(2) << term.termNumber << " [" << std::setw(2) << term.totalCredits << "] | ";

            const int bars = (term.totalCredits * BAR_WIDTH) / maxCredits;
            for (int i = 0; i < bars; ++i) md << BLOCK;
                md << "\n";
        }
        md << "```\n\n";
    }

        // Footer
        md << "---\n";
        md << "*Generated by Course Planner - HCMUTE Final Project*\n";

        return md.str();
    }

    EnhancedPlanResult Writer::loadFromJson(const string &filepath)
    {
        ifstream file(filepath);
        if (!file.is_open())
        {
            throw runtime_error("Cannot open file for reading: " + filepath);
        }

        json j;
        file >> j;
        file.close();

        EnhancedPlanResult result;

        // Parse metadata
        result.feasible = j["metadata"]["feasible"];
        result.generatedAt = j["metadata"]["generatedAt"];
        result.totalTermsUsed = j["metadata"]["totalTermsUsed"];
        result.totalCredits = j["metadata"]["totalCredits"];

        // Parse constraints
        result.constraints.numTerms = j["constraints"]["numTerms"];
        result.constraints.maxCreditsPerTerm = j["constraints"]["maxCreditsPerTerm"];
        result.constraints.minCreditsPerTerm = j["constraints"]["minCreditsPerTerm"];
        result.constraints.enforceCoreqTogether = j["constraints"]["enforceCoreqTogether"];

        // Parse terms
        for (const auto &termJson : j["terms"])
        {
            EnhancedPlanResult::TermInfo term;
            term.termNumber = termJson["termNumber"];
            term.totalCredits = termJson["totalCredits"];

            for (const auto &courseJson : termJson["courses"])
            {
                term.courseIds.push_back(courseJson["id"]);
                term.courseNames.push_back(courseJson["name"]);
                term.courseCredits.push_back(courseJson["credits"]);
            }

            result.terms.push_back(term);
        }

        // Parse optional fields
        if (j.contains("notes"))
        {
            for (const auto &note : j["notes"])
            {
                result.notes.push_back(note);
            }
        }

        if (j.contains("warnings"))
        {
            for (const auto &warning : j["warnings"])
            {
                result.warnings.push_back(warning);
            }
        }

        return result;
    }
}