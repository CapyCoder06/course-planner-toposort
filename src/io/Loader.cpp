/*
Đọc file JSON → parse thành Course, Curriculum, PlanConstraints.

Validate:
- Thiếu trường bắt buộc.
- Tín chỉ > 0.
- id không trùng.
- Tham chiếu prereq/coreq phải tồn tại.
- Nếu có offered_terms: nằm trong [1..numTerms].
- Thông báo lỗi có mã/ngữ cảnh (ví dụ: path khóa bị lỗi).

AC: Input hợp lệ → load ok; input lỗi → ném lỗi rõ ràng.

(Tùy chọn) src/io/Writer.h / Writer.cpp
API xuất kế hoạch ra JSON + Markdown.

AC: Có thể ghi ra terms, tổng tín chỉ/kỳ, và ghi chú.
*/

#include "Loader.h"
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace planner
{

    /**
     * @brief Nạp dữ liệu từ file JSON
     * @param filePath Đường dẫn tới file JSON
     * @return LoadResult gồm curriculum + constraints
     * @throws LoadException nếu có lỗi validate
     */
    LoadResult loadFromJsonFile(const std::string &filePath)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            throw LoadException(
                "Không thể mở file: " + filePath,
                "FILE_NOT_FOUND",
                filePath);
        }

        json j;
        try
        {
            file >> j;
        }
        catch (const json::parse_error &e)
        {
            throw LoadException(
                "Lỗi phân tích JSON: " + std::string(e.what()),
                "JSON_PARSE_ERROR",
                filePath + " tại byte " + std::to_string(e.byte));
        }

        return loadFromJson(j, filePath);
    }

    /**
     * @brief Nạp dữ liệu từ đối tượng JSON
     * @param j Đối tượng JSON cần parse
     * @param context Ngữ cảnh để báo lỗi (vd: tên file)
     * @return LoadResult gồm curriculum + constraints
     * @throws LoadException nếu dữ liệu không hợp lệ
     */
    LoadResult loadFromJson(const nlohmann::json &j, const std::string &context)
    {
        LoadResult result;

        // Kiểm tra cấu trúc cấp cao nhất
        validateRequired(j, {"courses", "constraints"}, context);

        // Parse constraints trước (cần để validate)
        result.constraints = parseConstraints(j["constraints"], context + ".constraints");

        // Parse danh sách môn học
        result.curriculum = parseCurriculum(j["courses"], result.constraints, context + ".courses");

        return result;
    }

    /**
     * @brief Parse ràng buộc kế hoạch từ JSON
     */
    PlanConstraints parseConstraints(const nlohmann::json &j, const std::string &context)
    {
        validateRequired(j, {"maxCreditsPerTerm", "minCreditsPerTerm", "numTerms"}, context);

        PlanConstraints constraints;

        constraints.maxCreditsPerTerm = parsePositiveInt(j["maxCreditsPerTerm"], context + ".maxCreditsPerTerm");
        constraints.minCreditsPerTerm = parsePositiveInt(j["minCreditsPerTerm"], context + ".minCreditsPerTerm");
        constraints.numTerms = parsePositiveInt(j["numTerms"], context + ".numTerms");

        // Các trường tùy chọn
        constraints.enforceCoreqTogether = j.value("enforceCoreqTogether", true);
        constraints.allowPartialLoads = j.value("allowPartialLoads", false);

        // Kiểm tra logic
        if (constraints.minCreditsPerTerm > constraints.maxCreditsPerTerm)
        {
            throw LoadException(
                "minCreditsPerTerm (" + std::to_string(constraints.minCreditsPerTerm) +
                    ") không thể lớn hơn maxCreditsPerTerm (" + std::to_string(constraints.maxCreditsPerTerm) + ")",
                "INVALID_CREDIT_RANGE",
                context);
        }

        return constraints;
    }

    /**
     * @brief Parse toàn bộ curriculum từ JSON
     */
    Curriculum parseCurriculum(const nlohmann::json &j, const PlanConstraints &constraints, const std::string &context)
    {
        if (!j.is_array())
        {
            throw LoadException("Trường courses phải là mảng", "INVALID_TYPE", context);
        }

        Curriculum curriculum;
        std::unordered_set<std::string> courseIds;

        // Lần 1: parse và kiểm tra trùng ID
        for (size_t i = 0; i < j.size(); ++i)
        {
            std::string courseContext = context + "[" + std::to_string(i) + "]";
            Course course = parseCourse(j[i], constraints, courseContext);

            if (courseIds.find(course.id) != courseIds.end())
            {
                throw LoadException(
                    "Trùng ID môn học: " + course.id,
                    "DUPLICATE_COURSE_ID",
                    courseContext + ".id");
            }

            courseIds.insert(course.id);
            curriculum.addCourse(std::move(course));
        }

        // Lần 2: kiểm tra tham chiếu (prereq/coreq)
        for (const auto &[id, course] : curriculum.courses)
        {
            std::string courseContext = context + "[" + id + "]";

            for (const auto &prereqId : course.prereq)
            {
                if (curriculum.courses.find(prereqId) == curriculum.courses.end())
                {
                    throw LoadException(
                        "Prerequisite không tồn tại: " + prereqId,
                        "UNKNOWN_PREREQUISITE",
                        courseContext + ".prereq");
                }
            }

            for (const auto &coreqId : course.coreq)
            {
                if (curriculum.courses.find(coreqId) == curriculum.courses.end())
                {
                    throw LoadException(
                        "Corequisite không tồn tại: " + coreqId,
                        "UNKNOWN_COREQUISITE",
                        courseContext + ".coreq");
                }
            }
        }

        return curriculum;
    }

    /**
     * @brief Parse một môn học từ JSON
     */
    Course parseCourse(const nlohmann::json &j, const PlanConstraints &constraints, const std::string &context)
    {
        validateRequired(j, {"id", "name", "credits"}, context);

        Course course;
        course.id = parseNonEmptyString(j["id"], context + ".id");
        course.name = parseNonEmptyString(j["name"], context + ".name");
        course.credits = parsePositiveInt(j["credits"], context + ".credits");

        course.prereq = parseStringArray(j, "prereq", context + ".prereq");
        course.coreq = parseStringArray(j, "coreq", context + ".coreq");

        if (j.contains("group"))
            course.group = j["group"].get<std::string>();
        if (j.contains("priority"))
            course.priority = j["priority"].get<int>();

        if (j.contains("offered_terms"))
        {
            course.offered_terms = parseIntArray(j, "offered_terms", context + ".offered_terms");
            for (int term : course.offered_terms)
            {
                if (term < 1 || term > constraints.numTerms)
                {
                    throw LoadException(
                        "offered_terms chứa giá trị không hợp lệ: " + std::to_string(term) +
                            " (phải nằm trong [1.." + std::to_string(constraints.numTerms) + "])",
                        "INVALID_OFFERED_TERM",
                        context + ".offered_terms");
                }
            }
        }

        return course;
    }

    // ====== Hàm tiện ích ======

    void validateRequired(const nlohmann::json &j, const std::vector<std::string> &requiredFields, const std::string &context)
    {
        for (const auto &field : requiredFields)
        {
            if (!j.contains(field))
            {
                throw LoadException(
                    "Thiếu trường bắt buộc: " + field,
                    "MISSING_FIELD",
                    context + "." + field);
            }
        }
    }

    std::string parseNonEmptyString(const nlohmann::json &j, const std::string &context)
    {
        if (!j.is_string())
            throw LoadException("Phải là string", "INVALID_TYPE", context);

        std::string value = j.get<std::string>();
        if (value.empty())
            throw LoadException("String không được rỗng", "EMPTY_STRING", context);

        return value;
    }

    int parsePositiveInt(const nlohmann::json &j, const std::string &context)
    {
        if (!j.is_number_integer())
            throw LoadException("Phải là số nguyên", "INVALID_TYPE", context);

        int value = j.get<int>();
        if (value <= 0)
            throw LoadException("Giá trị phải > 0, hiện tại = " + std::to_string(value),
                                "NON_POSITIVE_VALUE",
                                context);
        return value;
    }

    std::vector<std::string> parseStringArray(const nlohmann::json &j, const std::string &fieldName, const std::string &context)
    {
        std::vector<std::string> result;
        if (!j.contains(fieldName))
            return result;

        if (!j[fieldName].is_array())
            throw LoadException("Phải là mảng", "INVALID_TYPE", context);

        for (size_t i = 0; i < j[fieldName].size(); ++i)
        {
            std::string elemContext = context + "[" + std::to_string(i) + "]";
            result.push_back(parseNonEmptyString(j[fieldName][i], elemContext));
        }
        return result;
    }

    std::vector<int> parseIntArray(const nlohmann::json &j, const std::string &fieldName, const std::string &context)
    {
        std::vector<int> result;
        if (!j.contains(fieldName))
            return result;

        if (!j[fieldName].is_array())
            throw LoadException("Phải là mảng", "INVALID_TYPE", context);

        for (size_t i = 0; i < j[fieldName].size(); ++i)
        {
            std::string elemContext = context + "[" + std::to_string(i) + "]";
            if (!j[fieldName][i].is_number_integer())
                throw LoadException("Phải là số nguyên", "INVALID_TYPE", elemContext);

            result.push_back(j[fieldName][i].get<int>());
        }
        return result;
    }

}