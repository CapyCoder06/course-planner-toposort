#include <gtest/gtest.h>
#include "graph/CourseGraph.h"
#include "graph/TopoSort.h"
#include "graph/CycleDiagnosis.h"
#include "model/Course.h"
#include "model/Curriculum.h"
#include <unordered_set>
#include <algorithm>

static Curriculum makeCurriculum(const std::vector<Course> &courses)
{
    Curriculum curr;
    for (const auto &c : courses)
    {
        curr.add(c);
    }
    return curr;
}

TEST(GraphTopoTest, EmptyGraph)
{
    Curriculum curr = makeCurriculum({});
    CourseGraph g;
    g.build(curr);
    EXPECT_EQ(g.V, 0);
    EXPECT_TRUE(g.adj.empty());
    EXPECT_TRUE(g.indeg.empty());
    TopoResult res = topoSort(g);
    EXPECT_TRUE(res.success);
    EXPECT_EQ(res.order.size(), 0);
}
TEST(GraphTopoTest, SingleCourse)
{
    Course c1{"IP101", "Intro to Program", 3, {}, {}};
    Curriculum curr = makeCurriculum({c1});
    CourseGraph g;
    g.build(curr);
    EXPECT_EQ(g.V, 1);
    EXPECT_EQ(g.indeg[0], 0);
    EXPECT_TRUE(g.adj[0].empty());
    TopoResult res = topoSort(g);
    EXPECT_TRUE(res.success);
    ASSERT_EQ(res.order.size(), 1);
    EXPECT_EQ(res.order[0], 0);
}
TEST(GraphTopoTest, SimpleChain)
{
    Course c1{"IP101", "Intro to Program", 3, {}, {}};
    Course c2{"DS102", "Data Structures", 3, {"IP101"}, {}};
    Course c3{"AL201", "Algorithms", 3, {"DS102"}, {}};
    Curriculum curr = makeCurriculum({c1, c2, c3});
    CourseGraph g;
    g.build(curr);
    EXPECT_EQ(g.V, 3);
    EXPECT_EQ(g.indeg[0], 0);
    EXPECT_EQ(g.indeg[1], 1);
    EXPECT_EQ(g.indeg[2], 1);
    TopoResult res = topoSort(g);
    EXPECT_TRUE(res.success);
    ASSERT_EQ(res.order.size(), 3);
    EXPECT_EQ(res.order[0], 0);
    EXPECT_EQ(res.order[1], 1);
    EXPECT_EQ(res.order[2], 2);
}
TEST(GraphTopoTest, MultiSourceDiamond)
{
    Course c1{"IP101", "Intro to Program", 3, {}, {}};
    Course c2{"DS102", "Data Structures", 3, {"IP101"}, {}};
    Course c3{"MA101", "Calculus I", 3, {}, {}};
    Course c4{"AL201", "Algorithms", 3, {"DS102", "MA101"}, {}};
    Curriculum curr = makeCurriculum({c1, c2, c3, c4});
    CourseGraph g;
    g.build(curr);
    EXPECT_EQ(g.V, 4);
    EXPECT_EQ(g.indeg[0], 0);
    EXPECT_EQ(g.indeg[1], 1);
    EXPECT_EQ(g.indeg[2], 0);
    EXPECT_EQ(g.indeg[3], 2);
    TopoResult res = topoSort(g);
    EXPECT_TRUE(res.success);
    ASSERT_EQ(res.order.size(), 4);
    EXPECT_EQ(res.order[0], 0);
    EXPECT_EQ(res.order[1], 2);
    EXPECT_EQ(res.order[2], 1);
    EXPECT_EQ(res.order[3], 3);
}
TEST(GraphTopoTest, DisconnectedComponents)
{
    Course c1{"IP101", "Intro to Program", 3, {}, {}};
    Course c2{"DS102", "Data Structures", 3, {"IP101"}, {}};
    Course c3{"MA101", "Calculus I", 3, {}, {}};
    Course c4{"PH101", "Physics I", 3, {}, {}};
    Curriculum curr = makeCurriculum({c1, c2, c3, c4});
    CourseGraph g;
    g.build(curr);
    EXPECT_EQ(g.V, 4);
    EXPECT_EQ(g.indeg[0], 0);
    EXPECT_EQ(g.indeg[1], 1);
    EXPECT_EQ(g.indeg[2], 0);
    EXPECT_EQ(g.indeg[3], 0);
    TopoResult res = topoSort(g);
    EXPECT_TRUE(res.success);
    ASSERT_EQ(res.order.size(), 4);
    EXPECT_EQ(res.order[0], 0);
    EXPECT_EQ(res.order[1], 2);
    EXPECT_EQ(res.order[2], 3);
    EXPECT_EQ(res.order[3], 1);
}
TEST(GraphTopoTest, SimpleCycle)
{
    Course c1{"IP101", "Intro to Program", 3, {"AL201"}, {}};
    Course c2{"AL201", "Algorithms", 3, {"IP101"}, {}};
    Curriculum curr = makeCurriculum({c1, c2});
    CourseGraph g;
    g.build(curr);
    EXPECT_EQ(g.V, 2);
    TopoResult res = topoSort(g);
    EXPECT_FALSE(res.success);
    EXPECT_LT(res.order.size(), 2);
    std::vector<int> cycle = findOneCycle(g);
    EXPECT_FALSE(cycle.empty());
    EXPECT_GE(cycle.size(), 2);
    std::unordered_set<int> cycleSet(cycle.begin(), cycle.end());
    EXPECT_EQ(cycleSet.size(), 2);
}
TEST(GraphTopoTest, SelfLoop)
{
    Course c1{"IP101", "Intro to Program", 3, {"IP101"}, {}};
    Curriculum curr = makeCurriculum({c1});
    CourseGraph g;
    g.build(curr);
    EXPECT_EQ(g.V, 1);
    EXPECT_EQ(g.indeg[0], 1);
    TopoResult res = topoSort(g);
    EXPECT_FALSE(res.success);
    EXPECT_TRUE(res.order.empty());
    std::vector<int> cycle = findOneCycle(g);
    EXPECT_FALSE(cycle.empty());
}
TEST(GraphTopoTest, ComplexCycle)
{
    Course c1{"IP101", "Intro to Program", 3, {}, {}};
    Course c2{"DS102", "Data Structures", 3, {"IP101"}, {}};
    Course c3{"AL201", "Algorithms", 3, {"DS102"}, {}};
    Course c4{"MA101", "Calculus I", 3, {"AL201"}, {}};
    Course c5{"PH101", "Physics I", 3, {"MA101"}, {}};
    Curriculum curr = makeCurriculum({c1, c2, c3, c4, c5});
    CourseGraph g;
    g.build(curr);
    c5.prerequisite.push_back("DS102");
    g.build(curr);
    EXPECT_EQ(g.V, 5);
    TopoResult res = topoSort(g);
    EXPECT_FALSE(res.success);
    std::vector<int> cycle = findOneCycle(g);
    EXPECT_FALSE(cycle.empty());
    EXPECT_GE(cycle.size(), 2);
    for (size_t i = 0; i < cycle.size(); i++)
    {
        int u = cycle[i];
        int v = cycle[(i + 1) % cycle.size()];
        bool found = std::find(g.adj[u].begin(), g.adj[u].end(), v) != g.adj[u].end();
        EXPECT_TRUE(found) << "Expected edge from " << g.idxToId[u]
                           << " to " << g.idxToId[v];
    }
}
TEST(GraphTopoTest, BranchingStructure)
{
    Course c1{"IP101", "Intro to Program", 3, {}, {}};
    Course c2{"DS102", "Data Structures", 3, {"IP101"}, {}};
    Course c3{"AL201", "Algorithms", 3, {"IP101"}, {}};
    Course c4{"MA101", "Calculus I", 3, {"IP101"}, {}};
    Curriculum curr = makeCurriculum({c1, c2, c3, c4});
    CourseGraph g;
    g.build(curr);
    EXPECT_EQ(g.V, 4);
    int idxA = g.idToIdx.at("IP101");
    EXPECT_EQ(g.indeg[idxA], 0);
    EXPECT_EQ(g.adj[idxA].size(), 3);
    EXPECT_EQ(g.indeg[g.idToIdx.at("DS102")], 1);
    EXPECT_EQ(g.indeg[g.idToIdx.at("AL201")], 1);
    EXPECT_EQ(g.indeg[g.idToIdx.at("MA101")], 1);
    TopoResult res = topoSort(g);
    EXPECT_TRUE(res.success);
    ASSERT_EQ(res.order.size(), 4);
    EXPECT_EQ(res.order[0], idxA);
}
TEST(GraphTopoTest, UnknownPrerequisite)
{
    Course c{"IP101", "Intro to Program", 3, {"UNKNOWN"}, {}};
    Curriculum curr = makeCurriculum({c});
    CourseGraph g;
    EXPECT_THROW({ g.build(curr); }, std::runtime_error);
}
TEST(GraphTopoTest, DuplicateID)
{
    Course c1{"IP101", "Intro to Program", 3, {}, {}};
    Course c2{"IP101", "Data Structures", 3, {"IP101"}, {}};
    Curriculum curr;
    curr.add(c1);
    CourseGraph g;
    EXPECT_THROW({ curr.add(c2); g.build(curr); }, std::runtime_error);
}
TEST(GraphTopoTest, LargeDAG)
{
    int n = 1000;
    std::vector<Course> courses;
    for (int i = 0; i < n; i++)
    {
        Course a;
        a.id = "A" + std::to_string(i);
        a.name = "Course " + std::to_string(i);
        a.credits = 3;
        if (i > 0)
        {
            a.prerequisite.push_back("A" + std::to_string(i - 1));
        }
        courses.push_back(a);
    }
    Curriculum curr = makeCurriculum(courses);
    CourseGraph g;
    g.build(curr);
    EXPECT_EQ(g.V, n);
    TopoResult res = topoSort(g);
    EXPECT_TRUE(res.success);
    ASSERT_EQ(res.order.size(), n);
    for (int i = 0; i < n - 1; i++)
    {
        EXPECT_LT(res.order[i], res.order[i + 1]);
    }
}
TEST(GraphTopoTest, MultiplePrerequisites)
{
    Course c1{"IP101", "Intro to Program", 3, {}, {}};
    Course c2{"DS102", "Data Structures", 3, {}, {}};
    Course c3{"MA101", "Calculus I", 3, {}, {}};
    Course c4{"AL201", "Algorithms", 3, {"IP101", "DS102", "MA101"}, {}};
    Curriculum curr = makeCurriculum({c1, c2, c3, c4});
    CourseGraph g;
    g.build(curr);
    EXPECT_EQ(g.V, 4);
    int idxc4 = g.idToIdx.at("AL201");
    EXPECT_EQ(g.indeg[idxc4], 3);
    TopoResult res = topoSort(g);
    EXPECT_TRUE(res.success);
    ASSERT_EQ(res.order.size(), 4);
    EXPECT_EQ(res.order[3], idxc4);
}
