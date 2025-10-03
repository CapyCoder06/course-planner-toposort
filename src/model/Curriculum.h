#pragma once
#include <unordered_map>
#include "Course.h"
class Curriculum
{
public:
    bool exists(const std::string &courseId) const;

    const Course &get(const std::string &courseId) const;

    template <class Fn>
    void for_each(Fn &&fn) const
    {
        for (const auto &kv : courses)
        {
            fn(kv.second);
        }
    }

private:
    std::unordered_map<std::string, Course> courses;
};