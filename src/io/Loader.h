/*Khai báo hàm load dữ liệu từ JSON (hoặc YAML/CSV sau này).

Định nghĩa kiểu trả về gồm Curriculum + PlanConstraints.

AC: API rõ ràng, có mô tả các lỗi có thể ném ra.
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
#pragma once

#include <string>
#include <stdexcept>
#include <memory>
#include <vector>
#include "model/Curriculum.h"
#include "model/PlanConstraints.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
namespace planner
{
    struct LoadResult
    {
        Curriculum curriculum;
        PlanConstraints constraints;

        LoadResult() = default;
        LoadResult(Curriculum curr, PlanConstraints constr)
            : curriculum(std::move(curr)), constraints(std::move(constr)) {}
    };
    class LoadException : public std::runtime_error
    {
    private:
        std::string error_code_;
        std::string context_;

    public:
        LoadException(const std::string &message, const std::string &error_code = "", const std::string &context = "")
            : std::runtime_error(message), error_code_(error_code), context_(context) {}

        const std::string &getErrorCode() const noexcept { return error_code_; }
        const std::string &getContext() const noexcept { return context_; }
    };
    LoadResult loadFromJsonFile(const std::string &filePath);
    LoadResult loadFromJson(const nlohmann::json &j, const std::string &context = "");

    PlanConstraints parseConstraints(const nlohmann::json &j, const std::string &context);
    Curriculum parseCurriculum(const nlohmann::json &j, const PlanConstraints &constraints, const std::string &context);
    Course parseCourse(const nlohmann::json &j, const PlanConstraints &constraints, const std::string &context);

    void validateRequired(const nlohmann::json &j, const std::vector<std::string> &requiredFields, const std::string &context);
    std::string parseNonEmptyString(const nlohmann::json &j, const std::string &context);
    int parsePositiveInt(const nlohmann::json &j, const std::string &context);
    std::vector<std::string> parseStringArray(const nlohmann::json &j, const std::string &fieldName, const std::string &context);
    std::vector<int> parseIntArray(const nlohmann::json &j, const std::string &fieldName, const std::string &context);
}