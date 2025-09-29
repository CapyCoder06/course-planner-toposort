/*Khai báo hàm load dữ liệu từ JSON (hoặc YAML/CSV sau này).

Định nghĩa kiểu trả về gồm Curriculum + PlanConstraints.

AC: API rõ ràng, có mô tả các lỗi có thể ném ra.*/
#pragma once

#include <string>
#include <stdexcept>
#include <memory>
#include <vector>
#include "model/Curriculum.h"
#include "model/PlanConstraints.h"

namespace nlohmann
{
    class json;
}

namespace planner
{
    /**
     * @brief Kết quả trả về gồm chương trình học và ràng buộc kế hoạch
     */
    struct LoadResult
    {
        Curriculum curriculum;
        PlanConstraints constraints;

        LoadResult() = default;
        LoadResult(Curriculum curr, PlanConstraints constr)
            : curriculum(std::move(curr)), constraints(std::move(constr)) {}
    };

    /**
     * @brief Ngoại lệ ném ra khi load/parse dữ liệu có lỗi, kèm ngữ cảnh chi tiết
     */
    class LoadException : public std::runtime_error
    {
    private:
        std::string error_code_;
        std::string context_;

    public:
        /**
         * @param message Thông báo lỗi (dễ đọc cho con người)
         * @param error_code Mã lỗi (máy đọc được, ví dụ "MISSING_FIELD", "INVALID_CREDITS")
         * @param context Ngữ cảnh (ví dụ: đường dẫn trường, ID môn học)
         */
        LoadException(const std::string &message,
                      const std::string &error_code = "",
                      const std::string &context = "")
            : std::runtime_error(message), error_code_(error_code), context_(context) {}

        /// Lấy mã lỗi
        const std::string &getErrorCode() const noexcept { return error_code_; }

        /// Lấy ngữ cảnh lỗi (vd: vị trí JSON, ID môn học)
        const std::string &getContext() const noexcept { return context_; }
    };

    // ========== Hàm chính để nạp dữ liệu ==========

    /**
     * @brief Nạp chương trình học và ràng buộc từ file JSON
     * @param filePath Đường dẫn đến file JSON
     * @return LoadResult chứa dữ liệu chương trình học và ràng buộc
     * @throws LoadException khi có lỗi đọc file, lỗi JSON, hoặc lỗi validate
     */
    LoadResult loadFromJsonFile(const std::string &filePath);

    /**
     * @brief Nạp chương trình học và ràng buộc từ đối tượng JSON
     * @param j Đối tượng JSON
     * @param context Ngữ cảnh để báo lỗi (vd: đường dẫn file)
     * @return LoadResult chứa dữ liệu chương trình học và ràng buộc
     * @throws LoadException khi validate thất bại

     * Mã lỗi có thể gồm:
     * - MISSING_FIELD: Thiếu trường bắt buộc
     * - INVALID_TYPE: Trường có kiểu dữ liệu sai
     * - NON_POSITIVE_VALUE: Số phải > 0
     * - EMPTY_STRING: String rỗng
     * - INVALID_CREDIT_RANGE: minCreditsPerTerm > maxCreditsPerTerm
     * - DUPLICATE_COURSE_ID: Trùng ID môn học
     * - UNKNOWN_PREREQUISITE: Prereq không tồn tại
     * - UNKNOWN_COREQUISITE: Coreq không tồn tại
     * - INVALID_OFFERED_TERM: Giá trị offered_terms không hợp lệ
     */
    LoadResult loadFromJson(const nlohmann::json &j, const std::string &context = "");

    // ========== Hàm parser ==========

    /// Parse ràng buộc kế hoạch từ JSON
    PlanConstraints parseConstraints(const nlohmann::json &j, const std::string &context);

    /// Parse danh sách môn học từ JSON array
    Curriculum parseCurriculum(const nlohmann::json &j, const PlanConstraints &constraints, const std::string &context);

    /// Parse một môn học từ JSON object
    Course parseCourse(const nlohmann::json &j, const PlanConstraints &constraints, const std::string &context);

    // ========== Hàm tiện ích kiểm tra ==========

    /// Kiểm tra JSON có đủ trường bắt buộc
    void validateRequired(const nlohmann::json &j, const std::vector<std::string> &requiredFields, const std::string &context);

    /// Parse string không rỗng
    std::string parseNonEmptyString(const nlohmann::json &j, const std::string &context);

    /// Parse số nguyên dương
    int parsePositiveInt(const nlohmann::json &j, const std::string &context);

    /// Parse mảng string (tùy chọn)
    std::vector<std::string> parseStringArray(const nlohmann::json &j, const std::string &fieldName, const std::string &context);

    /// Parse mảng số nguyên (tùy chọn)
    std::vector<int> parseIntArray(const nlohmann::json &j, const std::string &fieldName, const std::string &context);

}