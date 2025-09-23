/*Định nghĩa cấu trúc môn học: id, tên, tín chỉ, danh sách tiên quyết, song hành, nhóm tự chọn (tùy), các thuộc tính mở rộng nếu cần (độ khó, độ ưu tiên, offered terms).

Thêm ghi chú về ràng buộc hợp lệ (credits > 0, id không trùng).

AC: Các trường đủ để mô hình hóa prerequisite/corequisite và ràng buộc mở môn.*/
#pragma once 
#include <vector>
#include <string>
#include <optional>
#include <unordered_set>
struct Course {
    std::string id;
    std::string name;
    unsigned short credits;
    std::vector<std::string> prereqisite;
    std::vector<std::string> coreqisite;
    std::optional<std::string> elective_groups;
    std::unordered_set<unsigned short> offered_terms;
};