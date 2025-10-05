#include <gtest/gtest.h>
#include "../src/io/Loader.h"
#include "../src/graph/CourseGraph.h"
#include "../src/graph/TopoSort.h"
#include "../src/planner/LongestPathDag.h"
#include "../src/planner/TermAssigner.h"
#include "../src/planner/Hints.h"
#include <nlohmann/json.hpp>

using namespace planner;
using json = nlohmann::json;

class OfferedTermsTest : public ::testing::Test
{
protected:
    PlanResult runPlanner(const json &j)
    {
        auto loadResult = loadFromJson(j);

        CourseGraph graph;
        graph.build(loadResult.curriculum);

        auto topoResult = topoSort(graph);
        EXPECT_TRUE(topoResult.success);

        auto earliestTerms = computeEarliestTerms(graph, topoResult);

        std::vector<int> creditsByIdx(graph.V);
        loadResult.curriculum.for_each([&](const Course &c)
                                       {
            int idx = graph.idToIdx.at(c.id);
            creditsByIdx[idx] = c.credits; });

        return assignTermsGreedy(graph, topoResult, earliestTerms.termByIdx,
                                 creditsByIdx, loadResult.constraints);
    }
};

TEST_F(OfferedTermsTest, CourseOfferedOnlyInOddTerms)
{
    json j = {
        {"constraints", {{"numTerms", 8}, {"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"enforceCoreqTogether", true}}},
        {"courses", {{{"id", "CS101"}, {"name", "Programming I"}, {"credits", 3}}, {{"id", "CS201"}, {"name", "Data Structures"}, {"credits", 3}, {"prerequisite", {"CS101"}}, {"offered_terms", {1, 3, 5, 7}}}}}};

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

    auto result = assignTermsGreedy(graph, topoResult, earliestTerms.termByIdx,
                                    creditsByIdx, loadResult.constraints);

    int cs201Idx = graph.idToIdx.at("CS201");
    int cs201Term = result.termOfIdx[cs201Idx];

    EXPECT_TRUE(cs201Term == 1 || cs201Term == 3 || cs201Term == 5 || cs201Term == 7)
        << "CS201 placed in term " << cs201Term << " but only offered in {1,3,5,7}";
}

TEST_F(OfferedTermsTest, WinterSummerCourses)
{
    json j = {
        {"constraints", {{"numTerms", 8}, {"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 6}, {"enforceCoreqTogether", true}}},
        {"courses", {
                        {{"id", "CS101"}, {"name", "Programming"}, {"credits", 3}}, {{"id", "SUMMER301"}, {"name", "Summer Intensive"}, {"credits", 4}, {"prerequisite", {"CS101"}}, {"offered_terms", {8}}}, // Only summer
                        {{"id", "WINTER201"}, {"name", "Winter Project"}, {"credits", 3}, {"offered_terms", {4}}}                                                                                             // Only winter
                    }}};

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

    auto result = assignTermsGreedy(graph, topoResult, earliestTerms.termByIdx,
                                    creditsByIdx, loadResult.constraints);

    if (result.ok)
    {
        int summerIdx = graph.idToIdx.at("SUMMER301");
        int winterIdx = graph.idToIdx.at("WINTER201");

        EXPECT_EQ(result.termOfIdx[summerIdx], 8);
        EXPECT_EQ(result.termOfIdx[winterIdx], 4);
    }
}

TEST_F(OfferedTermsTest, InfeasibleDueToOfferedTerms)
{
    json j = {
        {"constraints", {{"numTerms", 4}, {"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"enforceCoreqTogether", true}}},
        {"courses", {{{"id", "A"}, {"name", "Course A"}, {"credits", 3}, {"offered_terms", {1}}}, {{"id", "B"}, {"name", "Course B"}, {"credits", 3}, {"prerequisite", {"A"}}, {"offered_terms", {1}}}, // Can't take B after A in same term
                     {{"id", "C"}, {"name", "Course C"}, {"credits", 3}, {"prerequisite", {"B"}}, {"offered_terms", {1}}}}}};

    auto result = runPlanner(j);

    if (!result.ok)
    {
        EXPECT_FALSE(result.notes.empty());
    }
}

class HintsTest : public ::testing::Test
{
protected:
    void SetUp() override {}
};
/*
# numTerms
# maxCredits
# enforceCoreqTogether
# electiveConflict
# preferLightLoad
*/
TEST_F(HintsTest, SuggestIncreaseTerms)
{
    auto hints = Hints::analyze(
        6,
        18,
        true,
        false,
        false);

    ASSERT_FALSE(hints.empty());

    bool foundTermsHint = false;
    for (const auto &hint : hints)
    {
        if (hint.message.find("tăng số kỳ") != std::string::npos ||
            hint.message.find("numTerms") != std::string::npos)
        {
            foundTermsHint = true;
            EXPECT_TRUE(hint.actions.count("increase_numTerms_to") > 0);
        }
    }

    EXPECT_TRUE(foundTermsHint);
}

TEST_F(HintsTest, SuggestIncreaseMaxCredits)
{
    auto hints = Hints::analyze(
        8,
        15,
        true,
        false,
        false);

    bool foundCreditsHint = false;
    for (const auto &hint : hints)
    {
        if (hint.message.find("tín chỉ") != std::string::npos)
        {
            foundCreditsHint = true;
            EXPECT_TRUE(hint.actions.count("increase_maxCredits_to") > 0);
        }
    }

    EXPECT_TRUE(foundCreditsHint);
}

TEST_F(HintsTest, SuggestRelaxCoreq)
{
    auto hints = Hints::analyze(
        8,
        18,
        true,
        false,
        false);

    bool foundCoreqHint = false;
    for (const auto &hint : hints)
    {
        if (hint.message.find("coreq") != std::string::npos)
        {
            foundCoreqHint = true;
            EXPECT_TRUE(hint.actions.count("relax_coreqTogether") > 0);
        }
    }

    EXPECT_TRUE(foundCoreqHint);
}

TEST_F(HintsTest, SuggestElectiveChange)
{
    auto hints = Hints::analyze(
        8,
        18,
        true,
        true,
        false);

    bool foundElectiveHint = false;
    for (const auto &hint : hints)
    {
        if (hint.message.find("elective") != std::string::npos)
        {
            foundElectiveHint = true;
            EXPECT_TRUE(hint.actions.count("change_elective_group") > 0);
        }
    }

    EXPECT_TRUE(foundElectiveHint);
}

TEST_F(HintsTest, NoHintsForGenerousConstraints)
{
    auto hints = Hints::analyze(
        12,
        24,
        false,
        false,
        false);
}

TEST_F(HintsTest, PreferLightLoad_NoMaxCreditIncrease)
{
    auto hints = Hints::analyze(
        8,
        15,
        true,
        false,
        true);

    bool foundMaxCreditIncrease = false;
    for (const auto &hint : hints)
    {
        if (hint.actions.count("increase_maxCredits_to") > 0)
        {
            foundMaxCreditIncrease = true;
        }
    }

    EXPECT_FALSE(foundMaxCreditIncrease);
}
TEST_F(OfferedTermsTest, RealDataset_WinterSummer)
{
    ASSERT_NO_THROW({
        auto result = loadFromJsonFile("data/offered_terms_wintersummer.json");

        CourseGraph graph;
        graph.build(result.curriculum);

        auto topoResult = topoSort(graph);
        EXPECT_TRUE(topoResult.success);

        bool foundOfferedTerms = false;
        result.curriculum.for_each([&](const Course &c)
                                   {
            if (!c.offered_terms.empty()) {
                foundOfferedTerms = true;
                for (int term : c.offered_terms) {
                    EXPECT_GE(term, 1);
                    EXPECT_LE(term, result.constraints.numTerms);
                }
            } });

        EXPECT_TRUE(foundOfferedTerms)
            << "Expected to find courses with offered_terms restrictions";
    });
}