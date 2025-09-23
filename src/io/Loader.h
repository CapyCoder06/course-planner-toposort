/*Khai báo hàm load dữ liệu từ JSON (hoặc YAML/CSV sau này).

Định nghĩa kiểu trả về gồm Curriculum + PlanConstraints.

AC: API rõ ràng, có mô tả các lỗi có thể ném ra.*/
#pragma once

#include <string>
#include <stdexcept>
#include <memory>
#include "model/Curriculum.h"
#include "model/PlanConstraints.h"

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
        /**
         * @brief Construct LoadException with error details
         * @param message Human-readable error message
         * @param error_code Machine-readable error code (e.g., "MISSING_FIELD", "INVALID_CREDITS")
         * @param context Additional context (e.g., field path, course ID)
         */
        LoadException(const std::string &message,
                      const std::string &error_code = "",
                      const std::string &context = "")
            : std::runtime_error(message), error_code_(error_code), context_(context) {}

        const std::string &getErrorCode() const noexcept { return error_code_; }
        const std::string &getContext() const noexcept { return context_; }
    };
}