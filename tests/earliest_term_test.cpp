#include <gtest/gtest.h>
#include "../src/io/Loader.h"
#include "../src/graph/CourseGraph.h"
#include "../src/graph/TopoSort.h"
#include "../src/planner/LongestPathDag.h"
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <string>

using namespace planner;
using json = nlohmann::json;

class EarliestTermTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}

    std::unordered_map<std::string, int> computeEarliestTermsFromFile(const std::string &filepath)
    {
        auto result = loadFromJsonFile(filepath);

        CourseGraph graph;
        graph.build(result.curriculum);

        auto topoResult = topoSort(graph);
        EXPECT_TRUE(topoResult.success);

        auto earliestTerms = ::computeEarliestTerms(graph, topoResult);

        std::unordered_map<std::string, int> earliestTermById;
        result.curriculum.for_each([&](const Course &course)
                                   {
            int idx = graph.idToIdx.at(course.id);
            earliestTermById[course.id] = earliestTerms.termByIdx[idx]; });

        return earliestTermById;
    }
};

TEST_F(EarliestTermTest, Branching_OK)
{
    auto terms = computeEarliestTermsFromFile("tests/data/branching.json");

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
    auto terms = computeEarliestTermsFromFile("tests/data/multisources.json");

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
    auto terms = computeEarliestTermsFromFile("tests/data/disconnected.json");

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
        {"constraints", {{"numTerms", 4}, {"maxcredits", 18}, {"mincredits", 12}, {"enforceCoreqTogether", true}, {"offered_terms", json::array()}}},
        {"courses", json::array({{{"id", "CS101"},
                                  {"name", "Lập trình"},
                                  {"credits", 3},
                                  {"prerequisite", json::array()},
                                  {"corequisite", json::array()}}})}};

    auto result = loadFromJson(j);

    CourseGraph graph;
    graph.build(result.curriculum);

    auto topoResult = topoSort(graph);
    EXPECT_TRUE(topoResult.success);

    auto earliestTerms = ::computeEarliestTerms(graph, topoResult);

    int idx = graph.idToIdx.at("CS101");
    EXPECT_EQ(earliestTerms.termByIdx[idx], 1);
}

TEST_F(EarliestTermTest, DiamondPattern_OK)
{
    json j = {
        {"constraints", {{"numTerms", 6}, {"maxcredits", 18}, {"mincredits", 12}, {"enforceCoreqTogether", true}, {"offered_terms", json::array()}}},
        {"courses", json::array({{{"id", "A"},
                                  {"name", "Course A"},
                                  {"credits", 3},
                                  {"prerequisite", json::array()}},
                                 {{"id", "B"},
                                  {"name", "Course B"},
                                  {"credits", 3},
                                  {"prerequisite", json::array({"A"})}},
                                 {{"id", "C"},
                                  {"name", "Course C"},
                                  {"credits", 3},
                                  {"prerequisite", json::array({"A"})}},
                                 {{"id", "D"},
                                  {"name", "Course D"},
                                  {"credits", 3},
                                  {"prerequisite", json::array({"B", "C"})}}})}};

    auto result = loadFromJson(j);

    CourseGraph graph;
    graph.build(result.curriculum);

    auto topoResult = topoSort(graph);
    EXPECT_TRUE(topoResult.success);

    auto earliestTerms = ::computeEarliestTerms(graph, topoResult);

    EXPECT_EQ(earliestTerms.termByIdx[graph.idToIdx.at("A")], 1);
    EXPECT_EQ(earliestTerms.termByIdx[graph.idToIdx.at("B")], 2);
    EXPECT_EQ(earliestTerms.termByIdx[graph.idToIdx.at("C")], 2);
    EXPECT_EQ(earliestTerms.termByIdx[graph.idToIdx.at("D")], 3);
}

TEST_F(EarliestTermTest, EmptyCurriculum_OK)
{
    json j = {
        {"constraints", {{"numTerms", 4}, {"maxcredits", 18}, {"mincredits", 12}, {"enforceCoreqTogether", true}, {"offered_terms", json::array()}}},
        {"courses", json::array()}};

    auto result = loadFromJson(j);

    CourseGraph graph;
    graph.build(result.curriculum);

    auto topoResult = topoSort(graph);
    EXPECT_TRUE(topoResult.success);
    EXPECT_EQ(topoResult.order.size(), 0u);

    auto earliestTerms = ::computeEarliestTerms(graph, topoResult);
    EXPECT_EQ(earliestTerms.termByIdx.size(), 0u);
}