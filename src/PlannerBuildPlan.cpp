#include "PlannerService.h"
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <string>
#include <vector>

using namespace std;

// ======================= BUILD PLAN =======================
PlanResult PlannerService::buildPlan(Specialization spec, int maxCreditsPerTerm) const {
    PlanResult result;
    result.terms.resize(8);
    for (int i = 0; i < 8; ++i) result.terms[i].index = i + 1;

    // TODO:
    // - Sử dụng topoSort() từ PlannerGraph
    // - Dùng các hàm isGeneralTrack, isITCoreTrack, isSpecCore,... từ PlannerTracks
    // - Áp dụng logic gán môn vào các kỳ học (1–8)
    // - Trả về PlanResult hợp lệ

    result.ok = true;
    result.message = "Planning not yet implemented (placeholder).";
    return result;
}
