#include <gtest/gtest.h>
#include "../src/io/Loader.h"
#include "../src/graph/CourseGraph.h"
#include "../src/graph/TopoSort.h"
#include "../src/planner/LongestPathDag.h"
#include "../src/planner/TermAssigner.h"
#include <nlohmann/json.hpp>
#include <map>

using namespace planner;
using json = nlohmann::json;

class AssignerQuotaTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}

    PlanResult runAssigner(const json &j)
    {
        auto loadResult = loadFromJson(j);

        CourseGraph graph;
        graph.build(loadResult.curriculum);

        auto topoResult = topoSort(graph);
        EXPECT_TRUE(topoResult.success);

        auto earliestTerms = ::computeEarliestTerms(graph, topoResult);

        // Build creditsByIdx vector
        std::vector<int> creditsByIdx(graph.V);
        loadResult.curriculum.for_each([&](const Course &c)
                                       {
            int idx = graph.idToIdx.at(c.id);
            creditsByIdx[idx] = c.credits; });

        return assignTermsGreedy(graph, topoResult, earliestTerms.termByIdx,
                                 creditsByIdx, loadResult.constraints);
    }
};

TEST_F(AssignerQuotaTest, SimpleChain_RespectQuota)
{
    // Test: Simple chain should be assigned sequentially
    // Each course 3 credits, max 6 credits per term
    // Should result in 1 course per term
    json j = {
        {"constraints", {{"numTerms", 5}, {"maxcredits", 6}, {"mincredits", 3}, {"enforceCoreqTogether", true}, {"offered_terms", json::array()}}},
        {"courses", json::array({{{"id", "CS101"}, {"name", "Prog I"}, {"credits", 3}, {"prerequisite", json::array()}},
                                 {{"id", "CS102"}, {"name", "Prog II"}, {"credits", 3}, {"prerequisite", json::array({"CS101"})}},
                                 {{"id", "CS201"}, {"name", "Data Struct"}, {"credits", 3}, {"prerequisite", json::array({"CS102"})}},
                                 {{"id", "CS202"}, {"name", "Algorithms"}, {"credits", 3}, {"prerequisite", json::array({"CS201"})}}})}};

    auto result = runAssigner(j);

    EXPECT_TRUE(result.ok);

    // Count non-empty terms
    int nonEmptyTerms = 0;
    for (int term : result.termOfIdx)
    {
        if (term > 0)
            nonEmptyTerms++;
    }
    EXPECT_EQ(nonEmptyTerms, 4);
}

TEST_F(AssignerQuotaTest, ParallelCourses_FillTerms)
{
    // Test: Multiple independent courses should fill terms up to quota
    // 6 courses, 3 credits each, max 9 credits per term
    // Should pack 3 courses per term (2 terms)
    json j = {
        {"constraints", {{"numTerms", 4}, {"maxcredits", 9}, {"mincredits", 6}, {"enforceCoreqTogether", true}, {"offered_terms", json::array()}}},
        {"courses", json::array({{{"id", "MATH101"}, {"name", "Calculus I"}, {"credits", 3}, {"prerequisite", json::array()}},
                                 {{"id", "PHYS101"}, {"name", "Physics I"}, {"credits", 3}, {"prerequisite", json::array()}},
                                 {{"id", "CS101"}, {"name", "Programming"}, {"credits", 3}, {"prerequisite", json::array()}},
                                 {{"id", "ENG101"}, {"name", "English"}, {"credits", 3}, {"prerequisite", json::array()}},
                                 {{"id", "CHEM101"}, {"name", "Chemistry"}, {"credits", 3}, {"prerequisite", json::array()}},
                                 {{"id", "HIST101"}, {"name", "History"}, {"credits", 3}, {"prerequisite", json::array()}}})}};

    auto result = runAssigner(j);

    EXPECT_TRUE(result.ok);

    // Verify all courses are assigned
    int assignedCourses = 0;
    for (int term : result.termOfIdx)
    {
        if (term > 0)
            assignedCourses++;
    }
    EXPECT_EQ(assignedCourses, 6);
}

TEST_F(AssignerQuotaTest, Branching_ComplexDependencies)
{
    // Test: Branching structure with mixed prerequisites
    // Should respect both dependencies and quota
    json j = {
        {"constraints", {{"numTerms", 8}, {"maxcredits", 12}, {"mincredits", 6}, {"enforceCoreqTogether", true}, {"offered_terms", json::array()}}},
        {"courses", json::array({{{"id", "MATH101"}, {"name", "Calculus I"}, {"credits", 4}, {"prerequisite", json::array()}},
                                 {{"id", "MATH102"}, {"name", "Calculus II"}, {"credits", 4}, {"prerequisite", json::array({"MATH101"})}},
                                 {{"id", "CS101"}, {"name", "Prog Fundamentals"}, {"credits", 3}, {"prerequisite", json::array()}},
                                 {{"id", "CS201"}, {"name", "Data Structures"}, {"credits", 3}, {"prerequisite", json::array({"CS101"})}},
                                 {{"id", "CS202"}, {"name", "Algorithms"}, {"credits", 3}, {"prerequisite", json::array({"CS201", "MATH102"})}},
                                 {{"id", "PHYS101"}, {"name", "Physics I"}, {"credits", 4}, {"prerequisite", json::array({"MATH101"})}},
                                 {{"id", "STAT201"}, {"name", "Statistics"}, {"credits", 3}, {"prerequisite", json::array({"MATH101"})}}})}};

    auto loadResult = loadFromJson(j);
    CourseGraph graph;
    graph.build(loadResult.curriculum);

    auto topoResult = topoSort(graph);
    EXPECT_TRUE(topoResult.success);

    auto earliestTerms = ::computeEarliestTerms(graph, topoResult);

    std::vector<int> creditsByIdx(graph.V);
    loadResult.curriculum.for_each([&](const Course &c)
                                   {
        int idx = graph.idToIdx.at(c.id);
        creditsByIdx[idx] = c.credits; });

    auto result = assignTermsGreedy(graph, topoResult, earliestTerms.termByIdx,
                                    creditsByIdx, loadResult.constraints);

    EXPECT_TRUE(result.ok);

    // Check prerequisites are satisfied
    int math101_term = result.termOfIdx[graph.idToIdx.at("MATH101")];
    int math102_term = result.termOfIdx[graph.idToIdx.at("MATH102")];
    int cs101_term = result.termOfIdx[graph.idToIdx.at("CS101")];
    int cs201_term = result.termOfIdx[graph.idToIdx.at("CS201")];
    int cs202_term = result.termOfIdx[graph.idToIdx.at("CS202")];
    int phys101_term = result.termOfIdx[graph.idToIdx.at("PHYS101")];
    int stat201_term = result.termOfIdx[graph.idToIdx.at("STAT201")];

    EXPECT_LT(math101_term, math102_term);
    EXPECT_LT(cs101_term, cs201_term);
    EXPECT_LT(cs201_term, cs202_term);
    EXPECT_LT(math102_term, cs202_term);
    EXPECT_LT(math101_term, phys101_term);
    EXPECT_LT(math101_term, stat201_term);
}

TEST_F(AssignerQuotaTest, QuotaViolation_PushToNextTerm)
{
    // Test: Course that would violate quota should be pushed to next term
    // First course is 8 credits, second is 5 credits, max is 10
    // They cannot be in the same term
    json j = {
        {"constraints", {{"numTerms", 4}, {"maxcredits", 10}, {"mincredits", 5}, {"enforceCoreqTogether", true}, {"offered_terms", json::array()}}},
        {"courses", json::array({{{"id", "BIG1"}, {"name", "Big Course 1"}, {"credits", 8}, {"prerequisite", json::array()}},
                                 {{"id", "BIG2"}, {"name", "Big Course 2"}, {"credits", 5}, {"prerequisite", json::array()}}})}};

    auto loadResult = loadFromJson(j);
    CourseGraph graph;
    graph.build(loadResult.curriculum);

    auto topoResult = topoSort(graph);
    auto earliestTerms = ::computeEarliestTerms(graph, topoResult);

    std::vector<int> creditsByIdx(graph.V);
    loadResult.curriculum.for_each([&](const Course &c)
                                   {
        int idx = graph.idToIdx.at(c.id);
        creditsByIdx[idx] = c.credits; });

    auto result = assignTermsGreedy(graph, topoResult, earliestTerms.termByIdx,
                                    creditsByIdx, loadResult.constraints);

    EXPECT_TRUE(result.ok);

    int big1_term = result.termOfIdx[graph.idToIdx.at("BIG1")];
    int big2_term = result.termOfIdx[graph.idToIdx.at("BIG2")];

    // Should be in different terms
    EXPECT_NE(big1_term, big2_term);
}

TEST_F(AssignerQuotaTest, EmptyCurriculum_NoTerms)
{
    // Test: Empty curriculum should result in 0 terms
    json j = {
        {"constraints", {{"numTerms", 8}, {"maxcredits", 18}, {"mincredits", 12}, {"enforceCoreqTogether", true}, {"offered_terms", json::array()}}},
        {"courses", json::array()}};

    auto result = runAssigner(j);

    EXPECT_TRUE(result.ok);
    EXPECT_EQ(result.termOfIdx.size(), 0u);
}

TEST_F(AssignerQuotaTest, SingleCourse_OneTerm)
{
    // Test: Single course should be in term 1
    json j = {
        {"constraints", {{"numTerms", 8}, {"maxcredits", 18}, {"mincredits", 3}, {"enforceCoreqTogether", true}, {"offered_terms", json::array()}}},
        {"courses", json::array({{{"id", "CS101"}, {"name", "Programming"}, {"credits", 3}, {"prerequisite", json::array()}}})}};

    auto loadResult = loadFromJson(j);
    CourseGraph graph;
    graph.build(loadResult.curriculum);

    auto topoResult = topoSort(graph);
    auto earliestTerms = ::computeEarliestTerms(graph, topoResult);

    std::vector<int> creditsByIdx(graph.V);
    loadResult.curriculum.for_each([&](const Course &c)
                                   {
        int idx = graph.idToIdx.at(c.id);
        creditsByIdx[idx] = c.credits; });

    auto result = assignTermsGreedy(graph, topoResult, earliestTerms.termByIdx,
                                    creditsByIdx, loadResult.constraints);

    EXPECT_TRUE(result.ok);
    EXPECT_EQ(result.termOfIdx.size(), 1u);
    EXPECT_EQ(result.termOfIdx[0], 1);
}

TEST_F(AssignerQuotaTest, InfeasibleQuota_TooManyTerms)
{
    // Test: If we run out of terms, should be infeasible
    json j = {
        {"constraints", {{"numTerms", 2}, // Only 2 terms available
                         {"maxcredits", 6},
                         {"mincredits", 3},
                         {"enforceCoreqTogether", true},
                         {"offered_terms", json::array()}}},
        {"courses", json::array({{{"id", "C1"}, {"name", "Course 1"}, {"credits", 4}, {"prerequisite", json::array()}},
                                 {{"id", "C2"}, {"name", "Course 2"}, {"credits", 4}, {"prerequisite", json::array({"C1"})}},
                                 {{"id", "C3"}, {"name", "Course 3"}, {"credits", 4}, {"prerequisite", json::array({"C2"})}},
                                 {{"id", "C4"}, {"name", "Course 4"}, {"credits", 4}, {"prerequisite", json::array({"C3"})}}})} // Need 4 terms but only have 2
    };

    auto result = runAssigner(j);

    EXPECT_FALSE(result.ok);
    EXPECT_FALSE(result.notes.empty());
}