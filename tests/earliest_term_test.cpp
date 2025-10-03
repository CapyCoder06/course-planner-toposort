#include <gtest/gtest.h>
#include "../src/io/Loader.h"
#include "../src/graph/CourseGraph.h"
#include "../src/graph/TopoSort.h"
#include "../src/planner/LongestPathDag.h"
#include <unordered_map>
#include <string>

using namespace planner;
class EarliestTermTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
    std::unordered_map<std::string, int> computeEarliestTerms(const std::string &filepath)
    {
        auto result = loadFromJsonFile(filepath);
        CourseGraph graph(result.Curriculum);
        auto topoResult = topoSort(graph);
        EXPECT_TRUE(topoResult.success);
        uto earliestTermByIdx = longestPathDag(graph, topoResult.order);
        std::unordered_map<std::string, int> earliestTermById;
        result.curriculum.for_each([&](const Course &course)
                                   {
            int idx = graph.idxOf(course.id);
            earliestTermById[course.id] = earliestTermByIdx[idx]; });
        return earliestTermById;
    }
    TEST_F(EarliestTermTest, Branching_OK)
    {
        auto terms = computeEarliestTerms("tests/data/branching.json");
        EXPECT_EQ(terms["CS101"], 1);
        EXPECT_EQ(terms["CS102"], 2);
        EXPECT_EQ(terms["CS201"], 3);
        EXPECT_EQ(terms["CS202"], 3);
        EXPECT_EQ(terms["CS301"], 4);
        EXPECT_EQ(terms["CS302"], 4);
        EXPECT_EQ(terms["CS401"], 5);
    }
    TEST_F(EarliestTermTest, MultiSources_OK)
    {
        auto terms = computeEarliestTerms("tests/data/multisources.json");
        EXPECT_EQ(terms["MATH101"], 1);
        EXPECT_EQ(terms["CS101"], 1);
        EXPECT_EQ(terms["PHYS101"], 1);
        EXPECT_EQ(terms["MATH201"], 2);
        EXPECT_EQ(terms["CS201"], 2);
        EXPECT_EQ(terms["PHYS201"], 2);
        EXPECT_EQ(terms["CS301"], 3);
    }
    TEST_F(EarliestTermTest, Disconnected_OK)
    {
        auto terms = computeEarliestTerms("tests/data/disconnected.json");
        EXPECT_EQ(terms["CS101"], 1);
        EXPECT_EQ(terms["CS102"], 2);
        EXPECT_EQ(terms["CS201"], 3);
        EXPECT_EQ(terms["MATH101"], 1);
        EXPECT_EQ(terms["PHYS101"], 1);
        EXPECT_EQ(terms["ENG101"], 1);
        EXPECT_EQ(terms["ENG102"], 2);
    }
    TEST_F(EarliestTermTest, SingleCourse_OK)
    {
        json j = {
            {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 4}, {"enforceCoreqTogether", true}}},
            {"courses", {{{"id", "CS101"}, {"name", "Lập trình"}, {"credits", 3}, {"prerequisite", json::array()}, {"corequisite", json::array()}}}}};

        auto result = loadFromJson(j);
        CourseGraph graph(result.curriculum);
        auto topoResult = topoSort(graph);
        auto earliestTermByIdx = longestPathDag(graph, topoResult.order);
        int idx = graph.idxOf("CS101");
        EXPECT_EQ(earliestTermByIdx[idx], 1);
    }
    TEST_F(EarliestTermTest, DiamondPattern_OK)
    {
        json j = {
            {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 6}, {"enforceCoreqTogether", true}}},
            {"courses", {{{"id", "A"}, {"name", "Course A"}, {"credits", 3}, {"prerequisite", json::array()}}, {{"id", "B"}, {"name", "Course B"}, {"credits", 3}, {"prerequisite", {"A"}}}, {{"id", "C"}, {"name", "Course C"}, {"credits", 3}, {"prerequisite", {"A"}}}, {{"id", "D"}, {"name", "Course D"}, {"credits", 3}, {"prerequisite", {"B", "C"}}}}}};
        auto result = loadFromJson(j);
        CourseGraph graph(result.curriculum);
        auto topoResult = topoSort(graph);
        auto earliestTermByIdx = longestPathDag(graph, topoResult.order);
        EXPECT_EQ(earliestTermByIdx[graph.idxOf("A")], 1);
        EXPECT_EQ(earliestTermByIdx[graph.idxOf("B")], 2);
        EXPECT_EQ(earliestTermByIdx[graph.idxOf("C")], 2);
        EXPECT_EQ(earliestTermByIdx[graph.idxOf("D")], 3);
    }
    TEST_F(EarliestTermTest, EmptyCurriculum_OK)
    {
        json j = {
            {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 4}, {"enforceCoreqTogether", true}}},
            {"courses", json::array()}};

        auto result = loadFromJson(j);
        CourseGraph graph(result.curriculum);
        auto topoResult = topoSort(graph);

        EXPECT_TRUE(topoResult.success);
        EXPECT_EQ(topoResult.order.size(), 0u);

        auto earliestTermByIdx = longestPathDag(graph, topoResult.order);
        EXPECT_EQ(earliestTermByIdx.size(), 0u);
    }
};