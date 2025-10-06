#pragma once
#include <unordered_map>
#include <string>
#include <stdexcept>
#include "Course.h"

class Curriculum {
public:
    bool exists(const std::string& courseId) const {
        return courses.find(courseId) != courses.end();
    }

    const Course& get(const std::string& courseId) const {
        auto it = courses.find(courseId);
        if (it == courses.end()) {
            throw std::runtime_error("Curriculum::get: unknown course id: " + courseId);
        }
        return it->second;
    }

    void add(const Course& c) {
        courses[c.id] = c;
    }

    template <class Fn>
    void for_each(Fn&& fn) const {
        for (const auto& kv : courses) fn(kv.second);
    }

private:
    std::unordered_map<std::string, Course> courses;
};
