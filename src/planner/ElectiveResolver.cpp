#include "ElectiveResolver.h"
#include <algorithm>
#include <sstream>

using namespace std;

// chọn môn tự chọn dựa vào nhóm + độ ưu tiên
ElectiveResult ElectiveResolver::resolve(
    const vector<ElectiveGroup>& groups,
    const unordered_map<string, int>& creditTable,
    const unordered_map<string, vector<string>>& prereqTable
) {
    ElectiveResult result;
    result.feasible = true;

    // duyệt qua từng group để chọn môn
    for (const auto& group : groups) {
        vector<string> pool = group.courseIds;

        // sắp xếp: ưu tiên cao hơn trước, nếu bằng thì lấy môn ít tín chỉ hơn
        sort(pool.begin(), pool.end(), [&](const string& a, const string& b) {
            int pa = group.coursePriority.count(a) ? group.coursePriority.at(a) : 0;
            int pb = group.coursePriority.count(b) ? group.coursePriority.at(b) : 0;
            if (pa != pb) return pa > pb;

            int ca = creditTable.count(a) ? creditTable.at(a) : 0;
            int cb = creditTable.count(b) ? creditTable.at(b) : 0;
            return ca < cb;
        });

        // nếu không đủ số lượng môn thì coi như fail
        if ((int)pool.size() < group.requiredCount) {
            result.feasible = false;
            ostringstream oss;
            oss << "Group " << group.groupId << " chỉ có "
                << pool.size() << " môn, cần " << group.requiredCount;
            result.message += oss.str();
            return result;
        }

        // lấy mấy môn đầu trong danh sách đã sort
        for (int i = 0; i < group.requiredCount; ++i) {
            result.selectedCourses.insert(pool[i]);
        }
    }

    // kiểm tra xem có thiếu môn tiên quyết nào không
    if (!validatePrerequisites(result.selectedCourses, prereqTable)) {
        result.feasible = false;
        result.message += "Thiếu môn tiên quyết.";
    }

    return result;
}

// check prerequisite: nếu chọn 1 môn mà chưa có môn trước nó thì báo lỗi
bool ElectiveResolver::validatePrerequisites(
    const unordered_set<string>& chosen,
    const unordered_map<string, vector<string>>& prereqTable
) {
    for (const auto& entry : prereqTable) {
        const string& course = entry.first;
        const auto& prereqs = entry.second;

        if (chosen.count(course)) {
            for (const auto& pre : prereqs) {
                if (!chosen.count(pre)) {
                    return false; // thiếu 1 môn là fail luôn
                }
            }
        }
    }
    return true;
}
