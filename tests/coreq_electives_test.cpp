#include <gtest/gtest.h>
#include "../src/planner/Clusterizer.h"
#include "../src/planner/ElectiveResolver.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

using namespace std;

class ClusterizerTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(ClusterizerTest, SingleCoreqPair_OneCluster)
{
    unordered_map<string, int> courseCredits = {
        {"CS101", 3},
        {"LAB101", 1}};

    unordered_map<string, vector<string>> coreqs = {
        {"CS101", {"LAB101"}},
        {"LAB101", {"CS101"}}};

    int maxQuota = 10;

    Clusterizer clusterizer;
    auto result = clusterizer.buildClusters(courseCredits, coreqs, maxQuota);

    EXPECT_TRUE(result.feasible);
    EXPECT_EQ(result.clusterCredits.size(), 1u);
    EXPECT_EQ(result.clusterCredits[0], 4);

    EXPECT_EQ(result.courseToCluster["CS101"], result.courseToCluster["LAB101"]);
}

TEST_F(ClusterizerTest, ThreeCourseCoreqChain_OneCluster)
{
    unordered_map<string, int> courseCredits = {
        {"A", 2},
        {"B", 3},
        {"C", 2}};

    unordered_map<string, vector<string>> coreqs = {
        {"A", {"B"}},
        {"B", {"A", "C"}},
        {"C", {"B"}}};

    int maxQuota = 15;

    Clusterizer clusterizer;
    auto result = clusterizer.buildClusters(courseCredits, coreqs, maxQuota);

    EXPECT_TRUE(result.feasible);
    EXPECT_EQ(result.clusterCredits.size(), 1u);
    EXPECT_EQ(result.clusterCredits[0], 7);

    int clusterId = result.courseToCluster["A"];
    EXPECT_EQ(result.courseToCluster["B"], clusterId);
    EXPECT_EQ(result.courseToCluster["C"], clusterId);
}

TEST_F(ClusterizerTest, MultipleSeparateClusters)
{
    unordered_map<string, int> courseCredits = {
        {"CS101", 3}, {"LAB101", 1}, {"MATH101", 4}, {"MATHLAB", 1}, {"PHYS101", 3}};

    unordered_map<string, vector<string>> coreqs = {
        {"CS101", {"LAB101"}},
        {"LAB101", {"CS101"}},
        {"MATH101", {"MATHLAB"}},
        {"MATHLAB", {"MATH101"}}};

    int maxQuota = 10;

    Clusterizer clusterizer;
    auto result = clusterizer.buildClusters(courseCredits, coreqs, maxQuota);

    EXPECT_TRUE(result.feasible);
    EXPECT_EQ(result.clusterCredits.size(), 3u);

    EXPECT_EQ(result.courseToCluster["CS101"], result.courseToCluster["LAB101"]);

    EXPECT_EQ(result.courseToCluster["MATH101"], result.courseToCluster["MATHLAB"]);

    EXPECT_NE(result.courseToCluster["PHYS101"], result.courseToCluster["CS101"]);
    EXPECT_NE(result.courseToCluster["PHYS101"], result.courseToCluster["MATH101"]);
}

TEST_F(ClusterizerTest, ClusterExceedsQuota_Infeasible)
{
    unordered_map<string, int> courseCredits = {
        {"BIG1", 6},
        {"BIG2", 6}};

    unordered_map<string, vector<string>> coreqs = {
        {"BIG1", {"BIG2"}},
        {"BIG2", {"BIG1"}}};

    int maxQuota = 10;

    Clusterizer clusterizer;
    auto result = clusterizer.buildClusters(courseCredits, coreqs, maxQuota);

    EXPECT_FALSE(result.feasible);
    EXPECT_FALSE(result.note.empty());
    EXPECT_NE(result.note.find("vượt quota"), string::npos);
}

TEST_F(ClusterizerTest, NoCorequisites_AllSingleClusters)
{
    unordered_map<string, int> courseCredits = {
        {"A", 3},
        {"B", 3},
        {"C", 3}};

    unordered_map<string, vector<string>> coreqs;

    int maxQuota = 10;

    Clusterizer clusterizer;
    auto result = clusterizer.buildClusters(courseCredits, coreqs, maxQuota);

    EXPECT_TRUE(result.feasible);
    EXPECT_EQ(result.clusterCredits.size(), 3u);

    for (int credits : result.clusterCredits)
    {
        EXPECT_EQ(credits, 3);
    }
}

class ElectiveResolverTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(ElectiveResolverTest, SimpleElectiveGroup_Choose2of3)
{
    ElectiveGroup group;
    group.groupId = "TECH_ELECTIVES";
    group.courseIds = {"WEB101", "MOBILE101", "AI101"};
    group.requiredCount = 2;
    group.coursePriority = {
        {"AI101", 10},
        {"WEB101", 5},
        {"MOBILE101", 3}};

    unordered_map<string, int> creditTable = {
        {"WEB101", 3},
        {"MOBILE101", 3},
        {"AI101", 3}};

    unordered_map<string, vector<string>> prereqTable;

    ElectiveResolver resolver;
    auto result = resolver.resolve({group}, creditTable, prereqTable);

    EXPECT_TRUE(result.feasible);
    EXPECT_EQ(result.selectedCourses.size(), 2u);
    EXPECT_EQ(result.selectedCourses.count("AI101"), 1u);
    EXPECT_EQ(result.selectedCourses.count("WEB101"), 1u);
    EXPECT_EQ(result.selectedCourses.count("MOBILE101"), 0u);
}

TEST_F(ElectiveResolverTest, MultipleGroups_BothSatisfied)
{
    ElectiveGroup group1;
    group1.groupId = "MATH_ELECTIVES";
    group1.courseIds = {"CALC3", "DIFFEQ", "DISCRETE"};
    group1.requiredCount = 2;

    ElectiveGroup group2;
    group2.groupId = "LAB_ELECTIVES";
    group2.courseIds = {"WEBLAB", "MOBILELAB"};
    group2.requiredCount = 1;

    unordered_map<string, int> creditTable = {
        {"CALC3", 4}, {"DIFFEQ", 4}, {"DISCRETE", 3}, {"WEBLAB", 2}, {"MOBILELAB", 2}};

    unordered_map<string, vector<string>> prereqTable;

    ElectiveResolver resolver;
    auto result = resolver.resolve({group1, group2}, creditTable, prereqTable);

    EXPECT_TRUE(result.feasible);
    EXPECT_EQ(result.selectedCourses.size(), 3u);
}

TEST_F(ElectiveResolverTest, NotEnoughCourses_Infeasible)
{
    ElectiveGroup group;
    group.groupId = "SPARSE_GROUP";
    group.courseIds = {"A", "B"};
    group.requiredCount = 3;

    unordered_map<string, int> creditTable = {
        {"A", 3}, {"B", 3}};

    unordered_map<string, vector<string>> prereqTable;

    ElectiveResolver resolver;
    auto result = resolver.resolve({group}, creditTable, prereqTable);

    EXPECT_FALSE(result.feasible);
    EXPECT_FALSE(result.message.empty());
    EXPECT_NE(result.message.find("only has 2"), string::npos);
}

TEST_F(ElectiveResolverTest, PrerequisitesMissing_Infeasible)
{
    ElectiveGroup group;
    group.groupId = "ADVANCED";
    group.courseIds = {"ADV101", "ADV102", "ADV103"};
    group.requiredCount = 2;
    group.coursePriority = {
        {"ADV101", 10},
        {"ADV102", 5},
        {"ADV103", 3}};

    unordered_map<string, int> creditTable = {
        {"ADV101", 3}, {"ADV102", 3}, {"ADV103", 3}, {"BASIC101", 3}};

    unordered_map<string, vector<string>> prereqTable = {
        {"ADV101", {"BASIC101"}}};

    ElectiveResolver resolver;
    auto result = resolver.resolve({group}, creditTable, prereqTable);

    EXPECT_FALSE(result.feasible);
    EXPECT_FALSE(result.message.empty());
    EXPECT_NE(result.message.find("prerequisite"), string::npos);
}

TEST_F(ElectiveResolverTest, PriorityTieBreak_UseCredits)
{
    ElectiveGroup group;
    group.groupId = "MIXED";
    group.courseIds = {"LIGHT", "HEAVY", "MEDIUM"};
    group.requiredCount = 2;

    unordered_map<string, int> creditTable = {
        {"LIGHT", 2},
        {"HEAVY", 5},
        {"MEDIUM", 3}};

    unordered_map<string, vector<string>> prereqTable;

    ElectiveResolver resolver;
    auto result = resolver.resolve({group}, creditTable, prereqTable);

    EXPECT_TRUE(result.feasible);
    EXPECT_EQ(result.selectedCourses.size(), 2u);

    EXPECT_EQ(result.selectedCourses.count("LIGHT"), 1u);
    EXPECT_EQ(result.selectedCourses.count("MEDIUM"), 1u);
    EXPECT_EQ(result.selectedCourses.count("HEAVY"), 0u);
}

TEST_F(ElectiveResolverTest, EmptyGroup_NoSelection)
{
    unordered_map<string, int> creditTable;
    unordered_map<string, vector<string>> prereqTable;

    ElectiveResolver resolver;
    auto result = resolver.resolve({}, creditTable, prereqTable);

    EXPECT_TRUE(result.feasible);
    EXPECT_EQ(result.selectedCourses.size(), 0u);
}

TEST_F(ElectiveResolverTest, SelfSufficientPrereqs_Valid)
{
    ElectiveGroup group;
    group.groupId = "CHAIN";
    group.courseIds = {"BASE", "MID", "ADV"};
    group.requiredCount = 3;

    unordered_map<string, int> creditTable = {
        {"BASE", 3}, {"MID", 3}, {"ADV", 3}};

    unordered_map<string, vector<string>> prereqTable = {
        {"MID", {"BASE"}},
        {"ADV", {"MID"}}};

    ElectiveResolver resolver;
    auto result = resolver.resolve({group}, creditTable, prereqTable);

    EXPECT_TRUE(result.feasible);
    EXPECT_EQ(result.selectedCourses.size(), 3u);
}