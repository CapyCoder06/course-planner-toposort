#pragma once
#include <string>
#include <vector>

// Đọc JSON -> build CourseGraph -> gọi topoSort trong src/
std::vector<std::string> topoFromJsonFile(const std::string& path);
