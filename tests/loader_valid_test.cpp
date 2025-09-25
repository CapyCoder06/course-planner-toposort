#include <gtest/gtest.h>
#include "../src/io/Loader.h"
#include <nlohmann/json.hpp>

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
    // Kiểm tra nạp từ file JSON thực tế
    ASSERT_NO_THROW({
        auto result = loadFromJsonFile("data/sample_small.json");
        EXPECT_GE(result.curriculum.courses.size(), 1u);
        EXPECT_GT(result.constraints.maxCreditsPerTerm, 0);
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
        EXPECT_EQ(result.curriculum.courses.size(), 1u);
        EXPECT_TRUE(result.curriculum.exists("CS101"));

        const auto &course = result.curriculum.get("CS101");
        EXPECT_EQ(course.id, "CS101");
        EXPECT_EQ(course.name, "Nhập môn Lập trình");
        EXPECT_EQ(course.credits, 3);
        EXPECT_TRUE(course.prereq.empty());
        EXPECT_TRUE(course.coreq.empty());
    });
}

TEST_F(LoaderHopLeTest, MonCoTienQuyet)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", "CS101"}, {"name", "Lập trình I"}, {"credits", 3}}, {{"id", "CS102"}, {"name", "Lập trình II"}, {"credits", 3}, {"prereq", {"CS101"}}}}}};

    ASSERT_NO_THROW({
        auto result = loadFromJson(j);
        EXPECT_EQ(result.curriculum.courses.size(), 2u);

        const auto &cs102 = result.curriculum.get("CS102");
        EXPECT_EQ(cs102.prereq.size(), 1u);
        EXPECT_EQ(cs102.prereq[0], "CS101");
    });
}

TEST_F(LoaderHopLeTest, MonCoDongThoi)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", "MATH201"}, {"name", "Giải tích I"}, {"credits", 4}}, {{"id", "PHYS201"}, {"name", "Vật lý I"}, {"credits", 4}, {"coreq", {"MATH201"}}}}}};

    ASSERT_NO_THROW({
        auto result = loadFromJson(j);

        const auto &phys201 = result.curriculum.get("PHYS201");
        EXPECT_EQ(phys201.coreq.size(), 1u);
        EXPECT_EQ(phys201.coreq[0], "MATH201");
    });
}

TEST_F(LoaderHopLeTest, MonCoTruongTuyChon)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", "CS301"}, {"name", "Cấu trúc dữ liệu"}, {"credits", 3}, {"group", "core"}, {"priority", 10}, {"offered_terms", {1, 3, 5, 7}}}}}};

    ASSERT_NO_THROW({
        auto result = loadFromJson(j);

        const auto &course = result.curriculum.get("CS301");
        EXPECT_EQ(course.group.value(), "core");
        EXPECT_EQ(course.priority, 10);
        EXPECT_EQ(course.offered_terms.size(), 4u);
        EXPECT_EQ(course.offered_terms[0], 1);
        EXPECT_EQ(course.offered_terms[3], 7);
    });
}

TEST_F(LoaderHopLeTest, RangBuocCoTruongTuyChon)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 21}, {"minCreditsPerTerm", 9}, {"numTerms", 10}, {"enforceCoreqTogether", false}, {"allowPartialLoads", true}}},
        {"courses", {{{"id", "TEST101"}, {"name", "Môn kiểm thử"}, {"credits", 1}}}}};

    ASSERT_NO_THROW({
        auto result = loadFromJson(j);
        EXPECT_EQ(result.constraints.maxCreditsPerTerm, 21);
        EXPECT_EQ(result.constraints.minCreditsPerTerm, 9);
        EXPECT_EQ(result.constraints.numTerms, 10);
        EXPECT_FALSE(result.constraints.enforceCoreqTogether);
        EXPECT_TRUE(result.constraints.allowPartialLoads);
    });
}

TEST_F(LoaderHopLeTest, ChuoiPhuThuocPhucTap)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", "CS101"}, {"name", "Lập trình I"}, {"credits", 3}}, {{"id", "CS102"}, {"name", "Lập trình II"}, {"credits", 3}, {"prereq", {"CS101"}}}, {{"id", "CS201"}, {"name", "Cấu trúc dữ liệu"}, {"credits", 3}, {"prereq", {"CS102"}}}, {{"id", "CS301"}, {"name", "Thuật toán"}, {"credits", 3}, {"prereq", {"CS201"}}}}}};

    ASSERT_NO_THROW({
        auto result = loadFromJson(j);
        EXPECT_EQ(result.curriculum.courses.size(), 4u);
        EXPECT_EQ(result.curriculum.get("CS102").prereq.size(), 1u);
        EXPECT_EQ(result.curriculum.get("CS201").prereq.size(), 1u);
        EXPECT_EQ(result.curriculum.get("CS301").prereq.size(), 1u);
    });
}

TEST_F(LoaderHopLeTest, NhieuTienQuyet)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", "MATH101"}, {"name", "Giải tích I"}, {"credits", 4}}, {{"id", "CS101"}, {"name", "Lập trình I"}, {"credits", 3}}, {{"id", "CS201"}, {"name", "Tính toán khoa học"}, {"credits", 3}, {"prereq", {"MATH101", "CS101"}}}}}};

    ASSERT_NO_THROW({
        auto result = loadFromJson(j);

        const auto &cs201 = result.curriculum.get("CS201");
        EXPECT_EQ(cs201.prereq.size(), 2u);
        EXPECT_TRUE(std::find(cs201.prereq.begin(), cs201.prereq.end(), "MATH101") != cs201.prereq.end());
        EXPECT_TRUE(std::find(cs201.prereq.begin(), cs201.prereq.end(), "CS101") != cs201.prereq.end());
    });
}

TEST_F(LoaderHopLeTest, MangRongVanHopLe)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", "CS101"}, {"name", "Lập trình I"}, {"credits", 3}, {"prereq", json::array()}, {"coreq", json::array()}, {"offered_terms", json::array()}}}}};

    ASSERT_NO_THROW({
        auto result = loadFromJson(j);

        const auto &course = result.curriculum.get("CS101");
        EXPECT_TRUE(course.prereq.empty());
        EXPECT_TRUE(course.coreq.empty());
        EXPECT_TRUE(course.offered_terms.empty());
    });
}
