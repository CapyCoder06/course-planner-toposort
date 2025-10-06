#include <gtest/gtest.h>
#include "../src/io/Writer.h"
#include "../src/io/Loader.h"
#include "../src/graph/CourseGraph.h"
#include "../src/graph/TopoSort.h"
#include "../src/planner/LongestPathDag.h"
#include "../src/planner/TermAssigner.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

using namespace planner;
using json = nlohmann::json;
namespace fs = std::filesystem;

class WriterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        testOutputDir = "test_output";
        fs::create_directories(testOutputDir);
    }
    void TearDown() override
    {
        if (fs::exists(testOutputDir))
        {
            fs::remove_all(testOutputDir);
        }
    }

    EnhancedPlanResult createSampleResult()
    {
        json j = {
            {"constraints", {{"numTerms", 4}, {"maxCreditsPerTerm", 12}, {"minCreditsPerTerm", 6}, {"enforceCoreqTogether", true}}},
            {"courses", {{{"id", "CS101"}, {"name", "Programming I"}, {"credits", 3}}, {{"id", "CS102"}, {"name", "Programming II"}, {"credits", 3}, {"prerequisite", {"CS101"}}}, {{"id", "MATH101"}, {"name", "Calculus I"}, {"credits", 4}}, {{"id", "MATH102"}, {"name", "Calculus II"}, {"credits", 4}, {"prerequisite", {"MATH101"}}}}}};
        auto loadResult = loadFromJson(j);

        CourseGraph graph;
        graph.build(loadResult.curriculum);

        auto topoResult = topoSort(graph);
        auto earliestTerms = computeEarliestTerms(graph, topoResult);

        std::vector<int> creditsByIdx(graph.V);
        loadResult.curriculum.for_each([&](const Course &c)
                                       {
            int idx = graph.idToIdx.at(c.id);
            creditsByIdx[idx] = c.credits; });

        auto planResult = assignTermsGreedy(
            graph, topoResult, earliestTerms.termByIdx,
            creditsByIdx, loadResult.constraints);
        return Writer::enhance(planResult, graph, loadResult.curriculum,
                               loadResult.constraints);
    }
    std::string testOutputDir;
};
TEST_F(WriterTest, EnhanceBasic)
{
    auto enhanced = createSampleResult();

    EXPECT_TRUE(enhanced.feasible);
    EXPECT_GT(enhanced.totalTermsUsed, 0);
    EXPECT_GT(enhanced.totalCredits, 0);
    EXPECT_FALSE(enhanced.terms.empty());
}
TEST_F(WriterTest, JsonOutput_ValidFormat)
{
    auto enhanced = createSampleResult();

    std::string jsonStr = Writer::toJsonString(enhanced);

    ASSERT_NO_THROW({
        json j = json::parse(jsonStr);

        EXPECT_TRUE(j.contains("metadata"));
        EXPECT_TRUE(j.contains("constraints"));
        EXPECT_TRUE(j.contains("terms"));

        EXPECT_TRUE(j["metadata"].contains("feasible"));
        EXPECT_TRUE(j["metadata"].contains("totalTermsUsed"));
        EXPECT_TRUE(j["metadata"].contains("totalCredits"));
    });
}
TEST_F(WriterTest, JsonRoundTrip)
{
    auto original = createSampleResult();

    std::string filepath = testOutputDir + "/plan_roundtrip.json";
    Writer::writeJson(original, filepath);

    ASSERT_TRUE(fs::exists(filepath));

    auto loaded = Writer::loadFromJson(filepath);

    EXPECT_EQ(loaded.feasible, original.feasible);
    EXPECT_EQ(loaded.totalTermsUsed, original.totalTermsUsed);
    EXPECT_EQ(loaded.totalCredits, original.totalCredits);
    EXPECT_EQ(loaded.terms.size(), original.terms.size());
    EXPECT_EQ(loaded.constraints.numTerms, original.constraints.numTerms);
    EXPECT_EQ(loaded.constraints.maxCreditsPerTerm,
              original.constraints.maxCreditsPerTerm);

    for (size_t i = 0; i < loaded.terms.size(); ++i)
    {
        EXPECT_EQ(loaded.terms[i].termNumber, original.terms[i].termNumber);
        EXPECT_EQ(loaded.terms[i].totalCredits, original.terms[i].totalCredits);
        EXPECT_EQ(loaded.terms[i].courseIds.size(),
                  original.terms[i].courseIds.size());
    }
}
TEST_F(WriterTest, MarkdownOutput_HasStructure)
{
    auto enhanced = createSampleResult();

    std::string mdStr = Writer::toMarkdownString(enhanced);

    EXPECT_NE(mdStr.find("# Study Plan"), std::string::npos);
    EXPECT_NE(mdStr.find("## Summary"), std::string::npos);
    EXPECT_NE(mdStr.find("## Constraints"), std::string::npos);
    EXPECT_NE(mdStr.find("## Course Schedule"), std::string::npos);
    EXPECT_NE(mdStr.find("### Term"), std::string::npos);
    EXPECT_NE(mdStr.find("| Course ID |"), std::string::npos);
    EXPECT_NE(mdStr.find("| Credits |"), std::string::npos);
}

TEST_F(WriterTest, MarkdownFile_Created)
{
    auto enhanced = createSampleResult();

    std::string filepath = testOutputDir + "/plan.md";
    Writer::writeMarkdown(enhanced, filepath);

    ASSERT_TRUE(fs::exists(filepath));

    std::ifstream file(filepath);
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

    EXPECT_FALSE(content.empty());
    EXPECT_NE(content.find("Study Plan"), std::string::npos);
}
TEST_F(WriterTest, InfeasiblePlan_HasNotes)
{
    json j = {
        {"constraints", {{"numTerms", 2}, {"maxCreditsPerTerm", 6}, {"minCreditsPerTerm", 3}, {"enforceCoreqTogether", true}}},
        {"courses", {{{"id", "A"}, {"name", "Course A"}, {"credits", 4}}, {{"id", "B"}, {"name", "Course B"}, {"credits", 4}, {"prerequisite", {"A"}}}, {{"id", "C"}, {"name", "Course C"}, {"credits", 4}, {"prerequisite", {"B"}}}, {{"id", "D"}, {"name", "Course D"}, {"credits", 4}, {"prerequisite", {"C"}}}}}};

    auto loadResult = loadFromJson(j);
    CourseGraph graph;
    graph.build(loadResult.curriculum);

    auto topoResult = topoSort(graph);
    auto earliestTerms = computeEarliestTerms(graph, topoResult);

    std::vector<int> creditsByIdx(graph.V);
    loadResult.curriculum.for_each([&](const Course &c)
                                   {
        int idx = graph.idToIdx.at(c.id);
        creditsByIdx[idx] = c.credits; });

    auto planResult = assignTermsGreedy(
        graph, topoResult, earliestTerms.termByIdx,
        creditsByIdx, loadResult.constraints);

    auto enhanced = Writer::enhance(planResult, graph, loadResult.curriculum,
                                    loadResult.constraints);

    EXPECT_FALSE(enhanced.feasible);
    EXPECT_FALSE(enhanced.notes.empty());

    std::string jsonStr = Writer::toJsonString(enhanced);
    json j_out = json::parse(jsonStr);
    EXPECT_TRUE(j_out.contains("notes"));

    std::string mdStr = Writer::toMarkdownString(enhanced);
    EXPECT_NE(mdStr.find("## 📝 Notes"), std::string::npos);
}
TEST_F(WriterTest, CreditsDistribution_InMarkdown)
{
    auto enhanced = createSampleResult();

    std::string mdStr = Writer::toMarkdownString(enhanced);

    EXPECT_NE(mdStr.find("## Credits Distribution"), std::string::npos);
    EXPECT_NE(mdStr.find("```"), std::string::npos);
    EXPECT_NE(mdStr.find("Term"), std::string::npos);
}
TEST_F(WriterTest, MultipleTerms_CorrectOrder)
{
    auto enhanced = createSampleResult();

    for (size_t i = 1; i < enhanced.terms.size(); ++i)
    {
        EXPECT_LT(enhanced.terms[i - 1].termNumber, enhanced.terms[i].termNumber);
    }
}
TEST_F(WriterTest, CreditsCalculation_Correct)
{
    auto enhanced = createSampleResult();

    int calculatedTotal = 0;
    for (const auto &term : enhanced.terms)
    {
        calculatedTotal += term.totalCredits;

        int termSum = 0;
        for (int credits : term.courseCredits)
        {
            termSum += credits;
        }
        EXPECT_EQ(term.totalCredits, termSum);
    }
    EXPECT_EQ(enhanced.totalCredits, calculatedTotal);
}
TEST_F(WriterTest, Warnings_ForConstraintViolations)
{
    json j = {
        {"constraints", {{"numTerms", 2}, {"maxCreditsPerTerm", 8}, {"minCreditsPerTerm", 10}, {"enforceCoreqTogether", true}}},
        {"courses", {{{"id", "A"}, {"name", "Course A"}, {"credits", 3}}, {{"id", "B"}, {"name", "Course B"}, {"credits", 3}}}}};

    auto loadResult = loadFromJson(j);
    CourseGraph graph;
    graph.build(loadResult.curriculum);

    auto topoResult = topoSort(graph);
    auto earliestTerms = computeEarliestTerms(graph, topoResult);

    std::vector<int> creditsByIdx(graph.V);
    loadResult.curriculum.for_each([&](const Course &c)
                                   {
        int idx = graph.idToIdx.at(c.id);
        creditsByIdx[idx] = c.credits; });

    auto planResult = assignTermsGreedy(
        graph, topoResult, earliestTerms.termByIdx,
        creditsByIdx, loadResult.constraints);

    auto enhanced = Writer::enhance(planResult, graph, loadResult.curriculum,
                                    loadResult.constraints);

    EXPECT_FALSE(enhanced.warnings.empty());
}
TEST_F(WriterTest, EmptyPlan_HandledGracefully)
{
    json j = {
        {"constraints", {{"numTerms", 4}, {"maxCreditsPerTerm", 12}, {"minCreditsPerTerm", 6}, {"enforceCoreqTogether", true}}},
        {"courses", {}}};

    auto loadResult = loadFromJson(j);
    CourseGraph graph;
    graph.build(loadResult.curriculum);

    auto topoResult = topoSort(graph);
    auto earliestTerms = computeEarliestTerms(graph, topoResult);

    std::vector<int> creditsByIdx;

    auto planResult = assignTermsGreedy(
        graph, topoResult, earliestTerms.termByIdx,
        creditsByIdx, loadResult.constraints);

    auto enhanced = Writer::enhance(planResult, graph, loadResult.curriculum,
                                    loadResult.constraints);

    EXPECT_EQ(enhanced.totalTermsUsed, 0);
    EXPECT_EQ(enhanced.totalCredits, 0);
    EXPECT_TRUE(enhanced.terms.empty());

    ASSERT_NO_THROW({
        Writer::toJsonString(enhanced);
        Writer::toMarkdownString(enhanced);
    });
}
TEST_F(WriterTest, FileWriteError_ThrowsException)
{
    auto enhanced = createSampleResult();

    std::string invalidPath = "/invalid/path/that/does/not/exist/plan.json";

    EXPECT_THROW({ Writer::writeJson(enhanced, invalidPath); }, std::runtime_error);
}
TEST_F(WriterTest, FileReadError_ThrowsException)
{
    std::string nonExistentFile = testOutputDir + "/does_not_exist.json";

    EXPECT_THROW({ Writer::loadFromJson(nonExistentFile); }, std::runtime_error);
}
TEST_F(WriterTest, RealDataset_CompleteWorkflow)
{
    auto loadResult = loadFromJsonFile("data/branching.json");

    CourseGraph graph;
    graph.build(loadResult.curriculum);

    auto topoResult = topoSort(graph);
    ASSERT_TRUE(topoResult.success);

    auto earliestTerms = computeEarliestTerms(graph, topoResult);

    std::vector<int> creditsByIdx(graph.V);
    loadResult.curriculum.for_each([&](const Course &c)
                                   {
        int idx = graph.idToIdx.at(c.id);
        creditsByIdx[idx] = c.credits; });

    auto planResult = assignTermsGreedy(
        graph, topoResult, earliestTerms.termByIdx,
        creditsByIdx, loadResult.constraints);

    auto enhanced = Writer::enhance(planResult, graph, loadResult.curriculum,
                                    loadResult.constraints);
    std::string jsonPath = testOutputDir + "/branching_plan.json";
    std::string mdPath = testOutputDir + "/branching_plan.md";

    ASSERT_NO_THROW({
        Writer::writeJson(enhanced, jsonPath);
        Writer::writeMarkdown(enhanced, mdPath);
    });

    EXPECT_TRUE(fs::exists(jsonPath));
    EXPECT_TRUE(fs::exists(mdPath));

    auto loaded = Writer::loadFromJson(jsonPath);
    EXPECT_EQ(loaded.feasible, enhanced.feasible);
    EXPECT_EQ(loaded.terms.size(), enhanced.terms.size());
}