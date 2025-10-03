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
*/

#include "Loader.h"
#include "model/Course.h"
#include "model/Curriculum.h"
#include "model/PlanConstraints.h"
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace planner
{
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
    LoadResult loadFromJson(const nlohmann::json &j, const std::string &context)
    {
        LoadResult result;
        validateRequired(j, {"courses", "constraints"}, context);
        result.constraints = parseConstraints(j["constraints"], context + ".constraints");
        result.curriculum = parseCurriculum(j["courses"], result.constraints, context + ".courses");
        try
        {
            result.constraints.validate();
        }
        catch (const std::runtime_error &e)
        {
            throw LoadException(
                std::string(e.what()),
                "INVALID_CONSTRAINTS",
                context + ".constraints");
        }
        return result;
    }
    Curriculum parseCurriculum(const nlohmann::json &j, const PlanConstraints &constraints, const std::string &context)
    {
        if (!j.is_array())
        {
            throw LoadException("Trường courses phải là mảng", "INVALID_TYPE", context);
        }
        Curriculum curriculum;
        std::unordered_set<std::string> courseIds;
        std::vector<Course> parsedCourses;
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
            parsedCourses.push_back(std::move(course));
        }
        for (const auto &course : parsedCourses)
        {
            std::string courseContext = context + "[" + course.id + "]";

            for (const auto &prereqId : course.prerequisite)
            {
                if (courseIds.find(prereqId) == courseIds.end())
                {
                    throw LoadException(
                        "Prerequisite không tồn tại: " + prereqId,
                        "UNKNOWN_PREREQUISITE",
                        courseContext + ".prerequisite");
                }
            }
            for (const auto &coreqId : course.corequisite)
            {
                if (courseIds.find(coreqId) == courseIds.end())
                {
                    throw LoadException(
                        "Corequisite không tồn tại: " + coreqId,
                        "UNKNOWN_COREQUISITE",
                        courseContext + ".corequisite");
                }
            }
        }
        return curriculum;
    }
    Course parseCourse(const nlohmann::json &j, const PlanConstraints &constraints, const std::string &context)
    {
        validateRequired(j, {"id", "name", "credits"}, context);

        Course course;
        course.id = parseNonEmptyString(j["id"], context + ".id");
        course.name = parseNonEmptyString(j["name"], context + ".name");

        int credits = parsePositiveInt(j["credits"], context + ".credits");
        if (credits > 65535)
        {
            throw LoadException(
                "Số tín chỉ vượt quá giới hạn (65535): " + std::to_string(credits),
                "CREDITS_OVERFLOW",
                context + ".credits");
        }
        course.credits = static_cast<unsigned short>(credits);
        course.prerequisite = parseStringArray(j, "prerequisite", context + ".prerequisite");
        course.corequisite = parseStringArray(j, "corequisite", context + ".corequisite");
        if (j.contains("elective_groups") && !j["elective_groups"].is_null())
        {
            course.elective_groups = j["elective_groups"].get<std::string>();
        }
        if (j.contains("offered_terms"))
        {
            std::vector<int> terms = parseIntArray(j, "offered_terms", context + ".offered_terms");
            for (int term : terms)
            {
                if (term < 1 || term > constraints.numTerms)
                {
                    throw LoadException(
                        "offered_terms chứa giá trị không hợp lệ: " + std::to_string(term) +
                            " (phải nằm trong [1.." + std::to_string(constraints.numTerms) + "])",
                        "INVALID_OFFERED_TERM",
                        context + ".offered_terms");
                }
                if (term > 65535)
                {
                    throw LoadException(
                        "Giá trị term vượt quá giới hạn (65535): " + std::to_string(term),
                        "TERM_OVERFLOW",
                        context + ".offered_terms");
                }
                course.offered_terms.insert(static_cast<unsigned short>(term));
            }
        }
        return course;
    }

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