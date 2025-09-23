/*Định nghĩa tập môn học (map id → Course).

Ghi chú: đảm bảo tra cứu O(1) trung bình, duy trì tính nhất quán id.

AC: Có thể lặp qua tất cả môn & tra cứu theo id ổn định. */
#pragma once
#include <unordered_map>
#include "Course.h"
class Curriculum {
    public:
        bool exists(const std::string& courseId);

        const Course& get(const std::string& courseId);

        template<class Fn>
        void for_each(Fn && fn) {
            for (const auto& kv : courses) {
                fn(kv.second);
            }
        }

    private:
        std::unordered_map<std::string, Course> courses;
};