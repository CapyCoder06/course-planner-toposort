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

// Kiểm tra thiếu trường cấp cao nhất
TEST_F(LoaderKhongHopLeTest, ThieuTinChiMonHoc)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 28}, {"minCreditsPerTerm", 15}, {"numTerms", 8}}},
        {"courses", {{
                        {"id", "CS101"}, {"name", "Lập trình I"} // Thiếu "credits"
                    }}}};

    expectLoadException(j, "MISSING_FIELD");
}

// Kiểm tra giá trị không hợp lệ của môn học
TEST_F(LoaderKhongHopLeTest, IdMonHocRong)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", ""}, // Không hợp lệ: chuỗi rỗng
                      {"name", "Lập trình I"},
                      {"credits", 3}}}}};

    expectLoadException(j, "EMPTY_STRING");
}

TEST_F(LoaderKhongHopLeTest, TenMonHocRong)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", "CS101"}, {"name", ""}, // Không hợp lệ: chuỗi rỗng
                      {"credits", 3}}}}};

    expectLoadException(j, "EMPTY_STRING");
}

TEST_F(LoaderKhongHopLeTest, TinChiKhongDuong)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{
                        {"id", "CS101"}, {"name", "Lập trình I"}, {"credits", 0} // Không hợp lệ: không dương
                    }}}};

    expectLoadException(j, "NON_POSITIVE_VALUE");
}

TEST_F(LoaderKhongHopLeTest, TinChiAm)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{
                        {"id", "CS101"}, {"name", "Lập trình I"}, {"credits", -3} // Không hợp lệ: âm
                    }}}};

    expectLoadException(j, "NON_POSITIVE_VALUE");
}

// Kiểm tra ID trùng lặp
TEST_F(LoaderKhongHopLeTest, TrungIdMonHoc)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", "CS101"}, {"name", "Lập trình I"}, {"credits", 3}}, {{"id", "CS101"}, // ID trùng
                                                                                  {"name", "Lập trình II"},
                                                                                  {"credits", 3}}}}};

    expectLoadException(j, "DUPLICATE_COURSE_ID");
}

// Kiểm tra tiên quyết và đồng thời không tồn tại
TEST_F(LoaderKhongHopLeTest, TienQuyetKhongTonTai)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", "CS101"}, {"name", "Lập trình I"}, {"credits", 3}}, {
                                                                                     {"id", "CS102"}, {"name", "Lập trình II"}, {"credits", 3}, {"prereq", {"CS999"}} // Tiên quyết không tồn tại
                                                                                 }}}};

    expectLoadException(j, "UNKNOWN_PREREQUISITE");
}

TEST_F(LoaderKhongHopLeTest, DongThoiKhongTonTai)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", "CS101"}, {"name", "Lập trình I"}, {"credits", 3}}, {
                                                                                     {"id", "PHYS101"}, {"name", "Vật lý I"}, {"credits", 4}, {"coreq", {"MATH999"}} // Đồng thời không tồn tại
                                                                                 }}}};

    expectLoadException(j, "UNKNOWN_COREQUISITE");
}

// Kiểm tra offered_terms không hợp lệ
TEST_F(LoaderKhongHopLeTest, HocKyKhongHopLeNhoHon)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{
                        {"id", "CS101"}, {"name", "Lập trình I"}, {"credits", 3}, {"offered_terms", {0, 1, 2}} // 0 không hợp lệ (hợp lệ: 1-8)
                    }}}};

    expectLoadException(j, "INVALID_OFFERED_TERM");
}

TEST_F(LoaderKhongHopLeTest, HocKyKhongHopLeLonHon)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 4}}},
        {"courses", {{
                        {"id", "CS101"}, {"name", "Lập trình I"}, {"credits", 3}, {"offered_terms", {1, 2, 5}} // 5 không hợp lệ (hợp lệ: 1-4)
                    }}}};

    expectLoadException(j, "INVALID_OFFERED_TERM");
}

// Kiểm tra kiểu dữ liệu không hợp lệ
TEST_F(LoaderKhongHopLeTest, IdKhongPhaiChuoi)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", 101}, // Không hợp lệ: phải là chuỗi
                      {"name", "Lập trình I"},
                      {"credits", 3}}}}};

    expectLoadException(j, "INVALID_TYPE");
}

TEST_F(LoaderKhongHopLeTest, TinChiKhongPhaiSoNguyen)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{
                        {"id", "CS101"}, {"name", "Lập trình I"}, {"credits", "3"} // Không hợp lệ: phải là số nguyên
                    }}}};

    expectLoadException(j, "INVALID_TYPE");
}

TEST_F(LoaderKhongHopLeTest, PrereqKhongPhaiMang)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {{{"id", "CS101"}, {"name", "Lập trình I"}, {"credits", 3}}, {
                                                                                     {"id", "CS102"}, {"name", "Lập trình II"}, {"credits", 3}, {"prereq", "CS101"} // Không hợp lệ: phải là mảng
                                                                                 }}}};

    expectLoadException(j, "INVALID_TYPE");
}

TEST_F(LoaderKhongHopLeTest, CoursesKhongPhaiMang)
{
    json j = {
        {"constraints", {{"maxCreditsPerTerm", 18}, {"minCreditsPerTerm", 12}, {"numTerms", 8}}},
        {"courses", {// Không hợp lệ: phải là mảng, không phải object
                     {"id", "CS101"},
                     {"name", "Lập trình I"},
                     {"credits", 3}}}};

    expectLoadException(j, "INVALID_TYPE");
}

// Kiểm tra file không tồn tại
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