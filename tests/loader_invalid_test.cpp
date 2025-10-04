#include <gtest/gtest.h>
#include "../src/io/Loader.h"
#include <nlohmann/json.hpp>

using namespace planner;
using json = nlohmann::json;

class LoaderKhongHopLeTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}

    void expectLoadException(const json &j, const std::string &expectedErrorCode)
    {
        try
        {
            loadFromJson(j);
            FAIL() << "Mong đợi LoadException với mã lỗi: " << expectedErrorCode;
        }
        catch (const LoadException &e)
        {
            EXPECT_EQ(e.getErrorCode(), expectedErrorCode)
                << "Mong đợi mã lỗi: " << expectedErrorCode
                << ", nhận được: " << e.getErrorCode()
                << ", thông báo: " << e.what()
                << ", ngữ cảnh: " << e.getContext();
        }
    }
};
TEST_F(LoaderKhongHopLeTest, ThieuTinChiMonHoc)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 28}, {"minCreditsPerTerm", 15}, {"numTerms", 8}}},
        {"courses", {{{"id", "CS101"}, {"name", "Lập trình I"}}}}};

    expectLoadException(j, "MISSING_FIELD");
}
TEST_F(LoaderKhongHopLeTest, ThieuIdMonHoc)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"name", "Lập trình I"}, {"credits", 3}}}}};

    expectLoadException(j, "MISSING_FIELD");
}
TEST_F(LoaderKhongHopLeTest, ThieuTenMonHoc)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", "CS101"}, {"credits", 3}}}}};
    expectLoadException(j, "MISSING_FIELD");
}
TEST_F(LoaderKhongHopLeTest, IdMonHocRong)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", ""}, {"name", "Lập trình I"}, {"credits", 3}}}}};
    expectLoadException(j, "EMPTY_STRING");
}

TEST_F(LoaderKhongHopLeTest, TenMonHocRong)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", "CS101"}, {"name", ""}, {"credits", 3}}}}};

    expectLoadException(j, "EMPTY_STRING");
}
TEST_F(LoaderKhongHopLeTest, TinChiKhongDuong)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", "CS101"}, {"name", "Lập trình I"}, {"credits", 0}}}}};

    expectLoadException(j, "NON_POSITIVE_VALUE");
}
TEST_F(LoaderKhongHopLeTest, TinChiAm)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", "CS101"}, {"name", "Lập trình I"}, {"credits", -3}}}}};
    expectLoadException(j, "NON_POSITIVE_VALUE");
}
TEST_F(LoaderKhongHopLeTest, TrungIdMonHoc)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", "CS101"}, {"name", "Lập trình I"}, {"credits", 3}}, {{"id", "CS101"}, {"name", "Lập trình II"}, {"credits", 3}}}}};

    expectLoadException(j, "DUPLICATE_COURSE_ID");
}
TEST_F(LoaderKhongHopLeTest, TienQuyetKhongTonTai)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {
                        {{"id", "CS101"}, {"name", "Lập trình I"}, {"credits", 3}}, {{"id", "CS102"}, {"name", "Lập trình II"}, {"credits", 3}, {"prerequisite", {"CS999"}}} // Tiên quyết không tồn tại
                    }}};

    expectLoadException(j, "UNKNOWN_PREREQUISITE");
}
TEST_F(LoaderKhongHopLeTest, DongThoiKhongTonTai)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {
                        {{"id", "CS101"}, {"name", "Lập trình I"}, {"credits", 3}}, {{"id", "PHYS101"}, {"name", "Vật lý I"}, {"credits", 4}, {"corequisite", {"MATH999"}}} // Đồng thời không tồn tại
                    }}};

    expectLoadException(j, "UNKNOWN_COREQUISITE");
}
TEST_F(LoaderKhongHopLeTest, HocKyKhongHopLeNhoHon)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", "CS101"}, {"name", "Lập trình I"}, {"credits", 3}, {"offered_terms", {0, 1, 2}}}}}};
    expectLoadException(j, "INVALID_OFFERED_TERM");
}
TEST_F(LoaderKhongHopLeTest, HocKyKhongHopLeLonHon)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 4}}},
        {"courses", {{{"id", "CS101"}, {"name", "Lập trình I"}, {"credits", 3}, {"offered_terms", {1, 2, 5}}}}}};

    expectLoadException(j, "INVALID_OFFERED_TERM");
}
TEST_F(LoaderKhongHopLeTest, IdKhongPhaiChuoi)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", 101}, {"name", "Lập trình I"}, {"credits", 3}}}}};

    expectLoadException(j, "INVALID_TYPE");
}
TEST_F(LoaderKhongHopLeTest, TenKhongPhaiChuoi)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", "CS101"}, {"name", 12345}, {"credits", 3}}}}};

    expectLoadException(j, "INVALID_TYPE");
}
TEST_F(LoaderKhongHopLeTest, TinChiKhongPhaiSoNguyen)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", "CS101"}, {"name", "Lập trình I"}, {"credits", "3"}}}}};

    expectLoadException(j, "INVALID_TYPE");
}
TEST_F(LoaderKhongHopLeTest, PrereqKhongPhaiMang)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {
                        {{"id", "CS101"}, {"name", "Lập trình I"}, {"credits", 3}}, {{"id", "CS102"}, {"name", "Lập trình II"}, {"credits", 3}, {"prerequisite", "CS101"}} // Không hợp lệ: phải là mảng
                    }}};

    expectLoadException(j, "INVALID_TYPE");
}
TEST_F(LoaderKhongHopLeTest, CoreqKhongPhaiMang)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {
                        {{"id", "CS101"}, {"name", "Lập trình I"}, {"credits", 3}}, {{"id", "PHYS101"}, {"name", "Vật lý I"}, {"credits", 4}, {"corequisite", "CS101"}} // Không hợp lệ: phải là mảng
                    }}};
    expectLoadException(j, "INVALID_TYPE");
}
TEST_F(LoaderKhongHopLeTest, OfferedTermsKhongPhaiMang)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{
                        {"id", "CS101"}, {"name", "Lập trình I"}, {"credits", 3}, {"offered_terms", 1} // Không hợp lệ: phải là mảng
                    }}}};

    expectLoadException(j, "INVALID_TYPE");
}
TEST_F(LoaderKhongHopLeTest, CoursesKhongPhaiMang)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{"id", "CS101"}, {"name", "Lập trình I"}, {"credits", 3}}}};

    expectLoadException(j, "INVALID_TYPE");
}
TEST_F(LoaderKhongHopLeTest, ThieuTruongConstraints)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}}},
        {"courses", {{{"id", "CS101"}, {"name", "Lập trình I"}, {"credits", 3}}}}};

    expectLoadException(j, "MISSING_FIELD");
}
TEST_F(LoaderKhongHopLeTest, MaxCreditsNhoHonMinCredits)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 10}, {"minCreditsPerTerm", 15}, {"numTerms", 8}}},
        {"courses", {{{"id", "CS101"}, {"name", "Lập trình I"}, {"credits", 3}}}}};

    expectLoadException(j, "INVALID_CONSTRAINT");
}
TEST_F(LoaderKhongHopLeTest, NumTermsKhongDuong)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 0}}},
        {"courses", {{{"id", "CS101"}, {"name", "Lập trình I"}, {"credits", 3}}}}};

    expectLoadException(j, "NON_POSITIVE_VALUE");
}
TEST_F(LoaderKhongHopLeTest, FileKhongTonTai)
{
    try
    {
        loadFromJsonFile("khong_ton_tai.json");
        FAIL() << "Mong đợi LoadException với FILE_NOT_FOUND";
    }
    catch (const LoadException &e)
    {
        EXPECT_EQ(e.getErrorCode(), "FILE_NOT_FOUND");
        EXPECT_TRUE(e.getContext().find("khong_ton_tai.json") != std::string::npos);
    }
}