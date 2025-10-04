#include <gtest/gtest.h>
#include "../src/io/Loader.h"
#include <nlohmann/json.hpp>
#include <algorithm>

using namespace planner;
using json = nlohmann::json;

class LoaderHopLeTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(LoaderHopLeTest, NapFileMauNho)
{
    ASSERT_NO_THROW({
        auto result = loadFromJsonFile("data/sample_small.json");

        size_t courseCount = 0;
        result.curriculum.for_each([&courseCount](const Course &)
                                   { courseCount++; });

        EXPECT_GE(courseCount, 1u);
        EXPECT_GT(result.constraints.maxcredits, 0);
        EXPECT_GT(result.constraints.numTerms, 0);
    });
}
TEST_F(LoaderHopLeTest, ChuongTrinhToiThieu)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", "CS101"}, {"name", "Nhập môn Lập trình"}, {"credits", 3}}}}};
    ASSERT_NO_THROW({
        auto result = loadFromJson(j);

        size_t courseCount = 0;
        result.curriculum.for_each([&courseCount](const Course &)
                                   { courseCount++; });
        EXPECT_EQ(courseCount, 1u);
        EXPECT_TRUE(result.curriculum.exists("CS101"));
        const auto &course = result.curriculum.get("CS101");
        EXPECT_EQ(course.id, "CS101");
        EXPECT_EQ(course.name, "Nhập môn Lập trình");
        EXPECT_EQ(course.credits, 3);
        EXPECT_TRUE(course.prerequisite.empty());
        EXPECT_TRUE(course.corequisite.empty());
    });
}
TEST_F(LoaderHopLeTest, MonCoTienQuyet)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", "CS101"}, {"name", "Lập trình I"}, {"credits", 3}}, {{"id", "CS102"}, {"name", "Lập trình II"}, {"credits", 3}, {"prerequisite", {"CS101"}}}}}};
    ASSERT_NO_THROW({
        auto result = loadFromJson(j);
        size_t courseCount = 0;
        result.curriculum.for_each([&courseCount](const Course &)
                                   { courseCount++; });
        EXPECT_EQ(courseCount, 2u);
        const auto &cs102 = result.curriculum.get("CS102");
        EXPECT_EQ(cs102.prerequisite.size(), 1u);
        EXPECT_EQ(cs102.prerequisite[0], "CS101");
    });
}
TEST_F(LoaderHopLeTest, MonCoDongThoi)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", "MATH201"}, {"name", "Giải tích I"}, {"credits", 4}}, {{"id", "PHYS201"}, {"name", "Vật lý I"}, {"credits", 4}, {"corequisite", {"MATH201"}}}}}};

    ASSERT_NO_THROW({
        auto result = loadFromJson(j);

        const auto &phys201 = result.curriculum.get("PHYS201");
        EXPECT_EQ(phys201.corequisite.size(), 1u);
        EXPECT_EQ(phys201.corequisite[0], "MATH201");
    });
}
TEST_F(LoaderHopLeTest, MonCoTruongTuyChon)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", "CS301"}, {"name", "Cấu trúc dữ liệu"}, {"credits", 3}, {"elective_groups", "core"}, {"offered_terms", {1, 3, 5, 7}}}}}};
    ASSERT_NO_THROW({
        auto result = loadFromJson(j);

        const auto &course = result.curriculum.get("CS301");
        EXPECT_TRUE(course.elective_groups.has_value());
        EXPECT_EQ(course.elective_groups.value(), "core");
        EXPECT_EQ(course.offered_terms.size(), 4u);
        EXPECT_TRUE(course.offered_terms.count(1) > 0);
        EXPECT_TRUE(course.offered_terms.count(3) > 0);
        EXPECT_TRUE(course.offered_terms.count(5) > 0);
        EXPECT_TRUE(course.offered_terms.count(7) > 0);
    });
}
TEST_F(LoaderHopLeTest, RangBuocCoTruongTuyChon)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 21}, {"minCreditsPerTerm", 9}, {"numTerms", 10}, {"enforceCoreqTogether", false}}},
        {"courses", {{{"id", "TEST101"}, {"name", "Môn kiểm thử"}, {"credits", 1}}}}};

    ASSERT_NO_THROW({
        auto result = loadFromJson(j);
        EXPECT_EQ(result.constraints.maxcredits, 21);
        EXPECT_EQ(result.constraints.mincredits, 9);
        EXPECT_EQ(result.constraints.numTerms, 10);
        EXPECT_FALSE(result.constraints.enforceCoreqTogether);
    });
}
TEST_F(LoaderHopLeTest, ChuoiPhuThuocPhucTap)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", "CS101"}, {"name", "Lập trình I"}, {"credits", 3}}, {{"id", "CS102"}, {"name", "Lập trình II"}, {"credits", 3}, {"prerequisite", {"CS101"}}}, {{"id", "CS201"}, {"name", "Cấu trúc dữ liệu"}, {"credits", 3}, {"prerequisite", {"CS102"}}}, {{"id", "CS301"}, {"name", "Thuật toán"}, {"credits", 3}, {"prerequisite", {"CS201"}}}}}};

    ASSERT_NO_THROW({
        auto result = loadFromJson(j);
        size_t courseCount = 0;
        result.curriculum.for_each([&courseCount](const Course &)
                                   { courseCount++; });
        EXPECT_EQ(courseCount, 4u);
        EXPECT_EQ(result.curriculum.get("CS102").prerequisite.size(), 1u);
        EXPECT_EQ(result.curriculum.get("CS201").prerequisite.size(), 1u);
        EXPECT_EQ(result.curriculum.get("CS301").prerequisite.size(), 1u);
    });
}
TEST_F(LoaderHopLeTest, NhieuTienQuyet)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", "MATH101"}, {"name", "Giải tích I"}, {"credits", 4}}, {{"id", "CS101"}, {"name", "Lập trình I"}, {"credits", 3}}, {{"id", "CS201"}, {"name", "Tính toán khoa học"}, {"credits", 3}, {"prerequisite", {"MATH101", "CS101"}}}}}};

    ASSERT_NO_THROW({
        auto result = loadFromJson(j);

        const auto &cs201 = result.curriculum.get("CS201");
        EXPECT_EQ(cs201.prerequisite.size(), 2u);
        EXPECT_TRUE(std::find(cs201.prerequisite.begin(), cs201.prerequisite.end(), "MATH101") != cs201.prerequisite.end());
        EXPECT_TRUE(std::find(cs201.prerequisite.begin(), cs201.prerequisite.end(), "CS101") != cs201.prerequisite.end());
    });
}
TEST_F(LoaderHopLeTest, MangRongVanHopLe)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", "CS101"}, {"name", "Lập trình I"}, {"credits", 3}, {"prerequisite", json::array()}, {"corequisite", json::array()}, {"offered_terms", json::array()}}}}};

    ASSERT_NO_THROW({
        auto result = loadFromJson(j);

        const auto &course = result.curriculum.get("CS101");
        EXPECT_TRUE(course.prerequisite.empty());
        EXPECT_TRUE(course.corequisite.empty());
        EXPECT_TRUE(course.offered_terms.empty());
    });
}
TEST_F(LoaderHopLeTest, MonKhongCoTruongTuyChon)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", "CS101"}, {"name", "Lập trình I"}, {"credits", 3}}}}};

    ASSERT_NO_THROW({
        auto result = loadFromJson(j);

        const auto &course = result.curriculum.get("CS101");
        EXPECT_TRUE(course.prerequisite.empty());
        EXPECT_TRUE(course.corequisite.empty());
        EXPECT_FALSE(course.elective_groups.has_value());
        EXPECT_TRUE(course.offered_terms.empty());
    });
}
TEST_F(LoaderHopLeTest, NhieuDongThoi)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", "MATH201"}, {"name", "Giải tích I"}, {"credits", 4}}, {{"id", "MATH202"}, {"name", "Đại số"}, {"credits", 3}}, {{"id", "PHYS201"}, {"name", "Vật lý I"}, {"credits", 4}, {"corequisite", {"MATH201", "MATH202"}}}}}};
    ASSERT_NO_THROW({
        auto result = loadFromJson(j);

        const auto &phys201 = result.curriculum.get("PHYS201");
        EXPECT_EQ(phys201.corequisite.size(), 2u);
        EXPECT_TRUE(std::find(phys201.corequisite.begin(), phys201.corequisite.end(), "MATH201") != phys201.corequisite.end());
        EXPECT_TRUE(std::find(phys201.corequisite.begin(), phys201.corequisite.end(), "MATH202") != phys201.corequisite.end());
    });
}
TEST_F(LoaderHopLeTest, CaTienQuyetVaDongThoi)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", "CS101"}, {"name", "Lập trình I"}, {"credits", 3}}, {{"id", "MATH101"}, {"name", "Giải tích I"}, {"credits", 4}}, {{"id", "CS201"}, {"name", "Thuật toán"}, {"credits", 3}, {"prerequisite", {"CS101"}}, {"corequisite", {"MATH101"}}}}}};
    ASSERT_NO_THROW({
        auto result = loadFromJson(j);

        const auto &cs201 = result.curriculum.get("CS201");
        EXPECT_EQ(cs201.prerequisite.size(), 1u);
        EXPECT_EQ(cs201.prerequisite[0], "CS101");
        EXPECT_EQ(cs201.corequisite.size(), 1u);
        EXPECT_EQ(cs201.corequisite[0], "MATH101");
    });
}