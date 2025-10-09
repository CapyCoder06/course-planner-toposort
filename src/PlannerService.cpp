#include "PlannerService.h"

#include <nlohmann/json.hpp>
#include <algorithm>
#include <fstream>
#include <functional>
#include <queue>
#include <unordered_set>

using nlohmann::json;

// ======================= small utils =======================
static std::string up(std::string v) {
    for (auto& ch : v) ch = (char)toupper((unsigned char)ch);
    return v;
}

// Fill a term up to max credits with policy + optional preference
static void try_fill_to_max(
    int termIdx,
    int maxPerTerm,
    const std::vector<std::string>& topo,
    const std::function<bool(const std::string&)>& canPlace,
    const std::function<bool(const std::string&)>& fitsHere,
    const std::function<bool(const std::string&)>& allowHere,
    const std::function<void(int, const std::string&)>& place,
    std::function<bool(const std::string&)> prefer = nullptr
) {
    if (prefer) {
        for (const auto& id : topo) {
            if (!canPlace(id) || !allowHere(id) || !fitsHere(id)) continue;
            if (!prefer(id)) continue;
            place(termIdx, id);
        }
    }
    for (const auto& id : topo) {
        if (!canPlace(id) || !allowHere(id) || !fitsHere(id)) continue;
        place(termIdx, id);
    }
}

// ======================= tag/spec helpers =======================
bool PlannerService::isGeneralTrack(std::string t) {
    t = up(t);
    return t == "GENERAL" || t == "SUPPLEMENT"; // PATCH: coi AE (Supplement) là General để tự đặt
}


bool PlannerService::isITCoreTrack(std::string t) {
    t = up(t);
    return t == "ITCORE" || t == "IT CORE";
}

bool PlannerService::isITCoreElectiveTrack(std::string t) {
    if (t.empty()) return true; // nhiều môn elective để trống track
    t = up(t);
    return t == "IT CORE ELECTIVE" || t == "ITCOREELECTIVE" || t == "IT ELECTIVE";
}

std::string PlannerService::specKey(Specialization s) {
    switch (s) {
        case Specialization::SE:  return "SE";
        case Specialization::NNS: return "NNS";
        case Specialization::IS:  return "IS";
        case Specialization::AI:  return "AI";
        default:                  return "";
    }
}

bool PlannerService::isSpecCore(const Course& c, const std::string& skey) {
    auto tr = up(c.track);
    auto key = up(skey);
    return tr == ("SPEC:" + key);
}

bool PlannerService::isSpecElective(const Course& c, const std::string& skey) {
    auto tr = up(c.track);
    auto key = up(skey);
    return tr == key; // elective chuyên ngành
}

bool PlannerService::isSpecProjectId(const std::string& id, const std::string& skey) {
    auto key = up(skey);
    auto idU = up(id);
    if (key == "SE")  return idU == "POSE431479E";
    if (key == "NNS") return idU == "POCN431280E";
    if (key == "IS")  return idU == "POIS431184E";
    return false; // AI chưa có project riêng
}

bool PlannerService::isCapstoneId(const std::string& id) {
    return up(id) == "GRPR471979E";
}
// Specialized Elective theo chuyên ngành: nhận cả id prefix <SPEC>_SPEC_G*,
// track đúng chuyên ngành, hoặc tên chứa "Specialized Elective"
static inline bool isSpecElectiveForKey(const Course& c, const std::string& keyU) {
    const std::string idU = up(c.id);
    const std::string tr  = up(c.track);
    const std::string nm  = up(c.name);

    // id chuẩn nhóm: SE_SPEC_G1/G2, NNS_SPEC_G1/G2, IS_SPEC_G1/G2, AI_SPEC_G1/G2
    if (idU.rfind(keyU + "_SPEC_", 0) == 0) return true;

    // track = chuyên ngành (dataset đôi khi set track = "SE"/"NNS"/"IS"/"AI")
    if (tr == keyU && nm.find("SPECIALIZED ELECTIVE") != std::string::npos) return true;

    // tên chứa cụm "Specialized Elective (<SPEC>)"
    if (nm.find("SPECIALIZED ELECTIVE") != std::string::npos &&
        nm.find(keyU) != std::string::npos) return true;

    return false;
}


// ======= IT Core Elective whitelist (9 môn) =======
static const std::unordered_set<std::string> kItCoreElectiveIds = {
    "ADPL331379E", "ESYS431080E", "ITPM430884E", "ECOM430984E",
    "WESE431479E", "CLCO332779E", "INOT431780E", "DIPR430685E", "MALE431085E"
};
// Nhận diện IT Core Elective: kể cả các "môn nhóm" ITC_ELC_G*
static inline bool isItCoreElectiveId(const Course& c) {
    const std::string idU = up(c.id);
    if (kItCoreElectiveIds.count(idU)) return true;               // danh sách mã thật
    if (idU.rfind("ITC_ELC_", 0) == 0) return true;               // nhóm: ITC_ELC_G1..G4
    const std::string tr = up(c.track);
    if (tr == "IT CORE ELECTIVE" || tr == "ITCOREELECTIVE" || tr == "IT ELECTIVE") return true;
    const std::string nameU = up(c.name);
    if (nameU.find("IT CORE ELECTIVE") != std::string::npos) return true; // phòng khi track trống
    return false;
}

// Internship (xếp mặc định kỳ 7)
static inline bool isInternshipId(const std::string& idU) {
    return up(idU) == "ITIN441085E";
}

// Elective chuyên ngành theo prefix id (<SPEC>_SPEC_G*)
static inline bool idHasSpecPrefix(const std::string& id, const std::string& keyU) {
    const auto u = up(id);
    return u.rfind(keyU + "_SPEC_", 0) == 0;
}

// ======================= graph build / topo =======================
bool PlannerService::buildGraph(std::string& /*err*/) {
    adj_.clear();
    indeg_.clear();

    for (auto& [id, c] : courses_) {
        if (!indeg_.count(id)) indeg_[id] = 0;
        for (auto& pre : c.prereqs) {
            if (!courses_.count(pre)) continue; // bỏ CEFR/chứng chỉ
            adj_[pre].push_back(id);
            indeg_[id]++;
            if (!indeg_.count(pre)) indeg_[pre] = 0;
        }
    }
    for (auto& [id, _] : courses_) if (!adj_.count(id)) adj_[id] = {};
    return true;
}

bool PlannerService::topoSort(std::vector<std::string>& order, std::string& err) const {
    order.clear();
    auto deg = indeg_;
    
// Thuật toán 2.3: Cycle Detection (in-degree check inside Kahn) & 2.2: Kahn's Topological Sorting
std::queue<std::string> q;
    for (auto& [v, d] : deg) if (d == 0) q.push(v);
    while (!q.empty()) {
        auto u = q.front(); q.pop();
        order.push_back(u);
        auto it = adj_.find(u);
        if (it != adj_.end())
            for (auto& v : it->second)
                if (--deg[v] == 0) q.push(v);
    }
    if (order.size() != courses_.size()) { err = "Curriculum has cycles."; return false; }
    return true;
}

bool PlannerService::findCycle(std::vector<std::string>& cyc) const {
    cyc.clear();
    enum { W = 0, G = 1, B = 2 };
    std::unordered_map<std::string, int> col;
    std::unordered_map<std::string, std::string> par;
    for (auto& [id, _] : courses_) { col[id] = W; par[id] = ""; }

    std::function<bool(const std::string&)> dfs = [&](const std::string& u)->bool {
        col[u] = G;
        auto it = adj_.find(u);
        if (it != adj_.end()) for (auto& v : it->second) {
            if (!courses_.count(v)) continue;
            if (col[v] == W) { par[v] = u; if (dfs(v)) return true; }
            else if (col[v] == G) {
                std::vector<std::string> path; path.push_back(v);
                std::string cur = u;
                while (cur != v && !cur.empty()) { path.push_back(cur); cur = par[cur]; }
                path.push_back(v);
                std::reverse(path.begin(), path.end());
                cyc = std::move(path);
                return true;
            }
        }
        col[u] = B; return false;
    };

    for (auto& [id, _] : courses_) if (col[id] == W && dfs(id)) return true;
    return false;
}
// tìm id theo tên (không phân biệt hoa thường, trả về "" nếu không thấy)
static std::string findIdByNameContains(
    const std::unordered_map<std::string, Course>& courses,
    const std::string& needle)
{
    std::string upNeedle = up(needle);
    for (const auto& [id, c] : courses) {
        if (up(c.name).find(upNeedle) != std::string::npos) return id;
    }
    return "";
}

// true nếu là một môn Academic English bất kỳ
static bool isAcademicEnglish(const Course& c) {
    auto nm = up(c.name);
    if (nm.find("ACADEMIC ENGLISH") != std::string::npos) return true;
    // fallback: một số dataset dùng prefix ACEN****
    if (c.id.size() >= 4 && up(c.id.substr(0,4)) == "ACEN") return true;
    return false;
}

// phiên bản mạnh hơn: BEGINSWITH "Academic English 4" hoặc ACEN**** có hậu tố 4
static bool isAcademicEnglish4(const Course& c) {
    auto nm = up(c.name);
    if (nm.find("ACADEMIC ENGLISH 4") != std::string::npos) return true;
    // fallback: id chẵn (…735E/…835E…) thì khó đoán; ưu tiên theo name
    return false;
}

// Trả về index kỳ đã đặt 0..7, -1 nếu chưa đặt
static int termIndexOfInPlan(const PlanResult& R, const std::string& id) {
    for (int t = 0; t < (int)R.terms.size(); ++t)
        for (auto& pc : R.terms[t].courses)
            if (pc.id == id) return t;
    return -1;
}

// Kiểm tra: mọi tiên quyết của 'id' đã đặt ở kỳ < T
// Thuật toán 2.4: Earliest-Term Computation (đảm bảo mọi tiên quyết đặt ở kỳ < T)
bool prereqsOkByTerm(const PlanResult& R,
                     const std::unordered_map<std::string, Course>& courses,
                     const std::string& id,
                     int T) {
    auto it = courses.find(id);
    if (it == courses.end()) return false;
    for (const auto& pre : it->second.prereqs) {
        if (!courses.count(pre)) continue;     // bỏ qua label ngoại bảng
        int tp = termIndexOfInPlan(R, pre);
        if (tp == -1 || tp >= T) return false;
    }
    return true;
}

// Đếm số môn chuyên ngành đang có trong một kỳ
template<class IsSpecFn>
int countSpecInTerm(const PlanResult& R, int t, IsSpecFn isSpecAny) {
    int c = 0;
    for (auto& pc : R.terms[t].courses) if (isSpecAny(pc.id)) ++c;
    return c;
}


// ======================= load JSON =======================
bool PlannerService::loadCurriculum(const std::string& jsonPath, std::string& err) {
    err.clear();
    courses_.clear();
    adj_.clear();
    indeg_.clear();

    std::ifstream f(jsonPath);
    if (!f) { err = "Cannot open file: " + jsonPath; return false; }

    json j;
    try { f >> j; }
    catch (const std::exception& ex) { err = std::string("JSON parse error: ") + ex.what(); return false; }

    if (!j.contains("courses") || !j["courses"].is_array()) {
        err = "JSON must contain array 'courses'.";
        return false;
    }

    try {
        for (auto& x : j["courses"]) {
            Course c;
            c.id      = x.at("id").get<std::string>();
            c.name    = x.value("name", c.id);
            c.credits = x.value("credits", 0);
            c.track   = x.value("track", std::string{});
            if (x.contains("prereq") && x["prereq"].is_array()) {
                for (auto& p : x["prereq"]) c.prereqs.push_back(p.get<std::string>());
            }
            courses_[c.id] = std::move(c);
        }
    } catch (const std::exception& ex) {
        err = std::string("Invalid course entry: ") + ex.what();
        courses_.clear();
        return false;
    }

    return buildGraph(err);
}

// Trả về các mã tiên quyết là “mã môn” có trong curriculum (bỏ CEFR…)
std::vector<std::string> PlannerService::prereqsOf(const std::string& courseId) const {
    std::vector<std::string> out;
    auto it = courses_.find(courseId);
    if (it == courses_.end()) return out;
    for (const auto& p : it->second.prereqs) {
        if (courses_.count(p)) out.push_back(p);
    }
    return out;
}


// ======================= build plan =======================
PlanResult PlannerService::buildPlan(Specialization spec, int maxCreditsPerTerm) const {
    PlanResult R;
    R.terms.resize(8);
    for (int i = 0; i < 8; ++i) R.terms[i].index = i + 1;

    if (courses_.empty()) { R.ok = false; R.message = "No curriculum loaded."; return R; }

    // topo để duyệt “ứng viên” theo thứ tự hợp lý
    std::vector<std::string> topo;
    std::string err;
    if (!topoSort(topo, err)) {
        R.ok = false; R.message = err; return R;
    }

    // ==== tham số / helper nhận dạng nhóm môn ====
    const int MIN_PER_TERM = 15;
    const int MAX_PER_TERM = maxCreditsPerTerm;
    const std::string skey = specKey(spec);

    auto isGeneral    = [&](const std::string& id){ return isGeneralTrack(courses_.at(id).track); };
    auto isITCore     = [&](const std::string& id){ return isITCoreTrack(courses_.at(id).track); };
    auto isITCoreElec = [&](const std::string& id){ 
        const auto& c = courses_.at(id);
        if (kItCoreElectiveIds.count(c.id)) return true;
        // PATCH: không coi track rỗng là IT Core Elective
        // if (c.track.empty()) return true;
        return isITCoreElectiveTrack(c.track);
    };
    auto isSpecCoreFn = [&](const std::string& id){ return isSpecCore(courses_.at(id), skey); };
    auto isSpecElecFn = [&](const std::string& id){
        const auto& c = courses_.at(id);
        const auto keyU = up(skey);
        if (isSpecElectiveForKey(c, keyU)) return true; // tên chứa "Specialized Elective", prefix <SPEC>_SPEC_*
        return isSpecElective(c, skey) || idHasSpecPrefix(c.id, keyU);
    };
    auto isOtherSpecElec = [&](const std::string& id){
        const auto& c = courses_.at(id); const auto tr = up(c.track);
        if (tr=="SE"||tr=="NNS"||tr=="IS"||tr=="AI") return !isSpecElecFn(id);
        if (c.id.find("_SPEC_") != std::string::npos) return !isSpecElecFn(id);
        return false;
    };
    auto isSpecProjFn = [&](const std::string& id){ return isSpecProjectId(id, skey); };
    auto isCap        = [&](const std::string& id){ return isCapstoneId(id); };
    auto isIntern     = [&](const std::string& id){ return isInternshipId(id); };
    auto isSpecAny    = [&](const std::string& id){ return isSpecCoreFn(id) || isSpecElecFn(id) || isSpecProjFn(id); };

    // quota (IT Core elective 4, Spec elective <= thực tế)
    const int PICK_ITCORE_ELECTIVE = 4;
    int cntITCoreElec = 0, cntSpecElec = 0;

    // tình trạng đặt môn
    std::unordered_map<std::string, bool> placed; for (auto& [id,_] : courses_) placed[id] = false;

    auto fits = [&](int t, const std::string& id){
        return R.terms[t].totalCredits + courses_.at(id).credits <= MAX_PER_TERM;
    };
    auto place = [&](int t, const std::string& id){
        const auto& c = courses_.at(id);
        R.terms[t].courses.push_back({c.id, c.name, c.credits});
        R.terms[t].totalCredits += c.credits;
        placed[id] = true;
        if (isITCoreElec(id)) ++cntITCoreElec;
        if (isSpecElecFn(id)) ++cntSpecElec;
    };
auto canPlaceTerm = [&](int t, const std::string& id) -> bool {
    if (placed[id]) return false;
    // kiểm tra tiên quyết
    if (!prereqsOkByTerm(R, courses_, id, t)) return false;

    // kiểm tra trần tín chỉ chuẩn
    const int add = courses_.at(id).credits;
    if (R.terms[t].totalCredits + add > MAX_PER_TERM) return false;

    return true;
};


    // ====== Kỳ 1..4: General + IT Core + IT Core Elective ======
    for (int t = 0; t < 4; ++t) {
        // 1) General
        for (const auto& id : topo) if (isGeneral(id) && !isIntern(id) && up(id)!="PROJ215879E" && !isSpecProjectId(id, skey) && canPlaceTerm(t,id)) place(t,id);
        // 2) IT Core
        for (const auto& id : topo) if (isITCore(id) && canPlaceTerm(t,id)) place(t,id);
        // 3) IT Core Elective (tới khi đủ quota 4)
        for (const auto& id : topo) if (isITCoreElec(id) && cntITCoreElec < PICK_ITCORE_ELECTIVE && canPlaceTerm(t,id)) place(t,id);

        // 4) lấp thêm các môn không thuộc spec để đạt min
        for (const auto& id : topo) {
            if (isSpecAny(id) || isOtherSpecElec(id) || isIntern(id) || isCap(id) || isSpecProjectId(id, skey) || up(id)=="PROJ215879E") continue; // PATCH: không lấp IT Project vào K1..K4
            if (R.terms[t].totalCredits >= MIN_PER_TERM) break;
            if (canPlaceTerm(t,id)) place(t,id);
        }
    }

    { // ====== Kỳ 5: kết thúc phần chung (không đặt spec) ======
        int t = 4;
    // PATCH: đặt IT Project mặc định ở Kỳ 5 (index 4)
        {
            const std::string itProjId = "PROJ215879E";
            if (courses_.count(itProjId) && !placed[itProjId]) {
                const auto& c = courses_.at(itProjId);
                if (R.terms[t].totalCredits + c.credits <= MAX_PER_TERM &&
                    prereqsOkByTerm(R, courses_, itProjId, t)) {
                    R.terms[t].courses.push_back({c.id, c.name, c.credits});
                    R.terms[t].totalCredits += c.credits;
                    placed[itProjId] = true;
                }
            }
        }

        for (const auto& id : topo) if (isGeneral(id) && !isIntern(id) && up(id)!="PROJ215879E" && !isSpecProjectId(id, skey) && canPlaceTerm(t,id)) place(t,id);
        for (const auto& id : topo) if (isITCore(id) && canPlaceTerm(t,id)) place(t,id);
        for (const auto& id : topo) if (isITCoreElec(id) && cntITCoreElec < PICK_ITCORE_ELECTIVE && canPlaceTerm(t,id)) place(t,id);
        for (const auto& id : topo) { // lấp đủ min
            if (isSpecAny(id) || isOtherSpecElec(id) || isIntern(id) || isCap(id)) continue;
            if (R.terms[t].totalCredits >= MIN_PER_TERM) break;
            if (canPlaceTerm(t,id)) place(t,id);
        }
    } // ====== hết Kỳ 5 ======
    // (DISABLED per user) Bỏ ép buộc AE 1→4 vào 5 kỳ đầu
/*
// ====== ÉP ĐẶT ACADEMIC ENGLISH 1→4 vào các kỳ 1..5 ======
{
    const int FIRST_TERM = 0, LAST_TERM = 4; // kỳ 1..5 (0-based)
    // gom danh sách AE chưa xếp, theo thứ tự topo (đảm bảo prereq 1→2→3→4)
    std::vector<std::string> aeList;
    for (const auto& id : topo) {
        auto it = courses_.find(id); if (it == courses_.end()) continue;
        if (isAcademicEnglish(it->second)) {
            if (!placed[id]) aeList.push_back(id);
        }
    }

    auto fits = [&](int t, const std::string& id){
        return R.terms[t].totalCredits + courses_.at(id).credits <= MAX_PER_TERM;
    };
    auto canPlaceAE = [&](int t, const std::string& id){
        if (placed[id]) return false;
        if (!fits(t, id)) return false;
        if (!prereqsOkByTerm(R, courses_, id, t)) return false;
        return true;
    };
    auto place = [&](int t, const std::string& id){
        const auto& c = courses_.at(id);
        R.terms[t].courses.push_back({c.id, c.name, c.credits});
        R.terms[t].totalCredits += c.credits;
        placed[id] = true;
    };

    for (const auto& aeId : aeList) {
        bool done = false;
        for (int t = FIRST_TERM; t <= LAST_TERM; ++t) {
            if (canPlaceAE(t, aeId)) { place(t, aeId); done = true; break; }
        }
        if (!done) {
            // Không đủ chỗ: di dời 1 môn không quan trọng ra sau để nhét AE
            for (int t = FIRST_TERM; t <= LAST_TERM && !done; ++t) {
                // thử dịch 1 môn không phải AE/Cap/Intern/Spec ra kỳ 6
                for (int i = (int)R.terms[t].courses.size()-1; i >= 0; --i) {
                    auto mov = R.terms[t].courses[i].id;
                    const auto& cmov = courses_.at(mov);
                    if (isAcademicEnglish(cmov)) continue;
                    if (isCapstoneId(mov) || isInternshipId(mov)) continue;
                    // không dịch môn Spec về trước kỳ 6 (ở đây ta đang trong 1..5 nên ok để đẩy ra sau)
                    // kiểm tra có thể đưa sang một kỳ từ 6..8 không
                    bool pushed = false;
                    for (int tt = 5; tt < 8 && !pushed; ++tt) {
                        if (R.terms[tt].totalCredits + cmov.credits > MAX_PER_TERM) continue;
                        if (!prereqsOkByTerm(R, courses_, mov, tt)) continue;
                        // move
                        R.terms[t].courses.erase(R.terms[t].courses.begin()+i);
                        R.terms[t].totalCredits -= cmov.credits;
                        R.terms[tt].courses.push_back({cmov.id, cmov.name, cmov.credits});
                        R.terms[tt].totalCredits += cmov.credits;
                        placed[mov] = true; // vốn đã true
                        pushed = true;
                    }
                    if (pushed && canPlaceAE(t, aeId)) { place(t, aeId); done = true; break; }
                }
            }
        }
    }
}
// Sau khi đã chạy K1..K5, nếu IT Core Elective < 4 thì cố gắng đặt bổ sung
if (cntITCoreElec < PICK_ITCORE_ELECTIVE) {
    for (int t = 0; t <= 4 && cntITCoreElec < PICK_ITCORE_ELECTIVE; ++t) {
        for (const auto& id : topo) {
            if (placed[id]) continue;
            if (!isITCoreElec(id)) continue;
            if (!canPlaceTerm(t, id)) continue;
            place(t, id);
            if (cntITCoreElec >= PICK_ITCORE_ELECTIVE) break;
        }
    }
}

*/
// ====== Kỳ 6 & 7: chuyên ngành ======

// PATCH: Đặt cố định cho chuyên ngành SE
if (up(skey) == "SE") {
    // danh sách môn cố định cho mỗi kỳ (0-based: 5 = Kỳ 6, 6 = Kỳ 7)
    const std::vector<std::pair<int, std::string>> seFixed = {
        {5, "OOSE330679E"}, // Object-Oriented Software Engineering (kỳ 6)
        {5, "DEPA330879E"}, // Design Patterns (kỳ 6)
        {5, "MOPR331279E"}, // Programming for Mobile Devices (kỳ 6)
        {6, "SOTE431079E"}, // Software Testing (kỳ 7)
        {6, "MTSE431179E"}, // Modern Technologies on SE (kỳ 7)
        {6, "POSE431479E"}  // Project on Software Engineering (kỳ 7)
    };
    for (const auto& kv : seFixed) {
        int term = kv.first; const std::string& cid = kv.second;
        if (courses_.count(cid)==0) continue;                 // bỏ qua nếu dataset không có
        if (placed[cid]) continue;                             // đã đặt ở đâu đó thì bỏ qua
        if (!prereqsOkByTerm(R, courses_, cid, term)) continue; // không phá tiên quyết
        const auto& c = courses_.at(cid);
        if (R.terms[term].totalCredits + c.credits > MAX_PER_TERM) continue; // không vượt trần
        R.terms[term].courses.push_back({c.id, c.name, c.credits});
        R.terms[term].totalCredits += c.credits;
        placed[cid] = true;
        if (isSpecElecFn(cid)) ++cntSpecElec;
    }
}
// PATCH: Cố định mặc định cho các chuyên ngành KHÁC (IS/NNS/AI, v.v.)
// Quy tắc: chọn đúng track = skey (core + elective, trừ project).
// - Kỳ 6: lấy tối đa 3 môn đầu theo thứ tự topo (thỏa tiên quyết).
// - Kỳ 7: đổ phần còn lại (thỏa tiên quyết) và đặt Project ở Kỳ 7.
if (up(skey) != "SE") {
    // lấy danh sách spec (không project) theo thứ tự topo
    std::vector<std::string> specList;
    for (const auto& id : topo) {
        if (placed[id]) continue;
        if (isSpecProjectId(id, skey)) continue;
        if (isSpecCoreFn(id) || isSpecElecFn(id)) specList.push_back(id);
    }
    auto try_place = [&](int term, const std::string& id)->bool{
        if (placed[id]) return false;
        if (!prereqsOkByTerm(R, courses_, id, term)) return false;
        const auto& c = courses_.at(id);
        if (R.terms[term].totalCredits + c.credits > MAX_PER_TERM) return false;
        R.terms[term].courses.push_back({c.id, c.name, c.credits});
        R.terms[term].totalCredits += c.credits;
        placed[id] = true;
        if (isSpecElecFn(id)) ++cntSpecElec;
        return true;
    };
    // Kỳ 6: đặt tối đa 3 môn
    int put6 = 0;
    for (const auto& id : specList) {
        if (put6 >= 3) break;
        if (try_place(5, id)) ++put6;
    }
    // Kỳ 7: đặt phần còn lại
    for (const auto& id : specList) {
        if (!placed[id]) { (void)try_place(6, id); }
    }
    // đặt Project ở Kỳ 7 nếu có và còn chưa đặt
    for (const auto& kv : courses_) {
        const std::string& cid = kv.first;
        if (isSpecProjectId(cid, skey) && !placed[cid]) {
            (void)try_place(6, cid);
            break;
        }
    }
}

for (int t = 5; t < 7; ++t) {
    // 6: chỉ Spec Core + Spec Elective
    if (t == 5) {
        for (const auto& id : topo) if (isSpecCoreFn(id) && canPlaceTerm(t,id)) place(t,id);
        for (const auto& id : topo) if (isSpecElecFn(id) && canPlaceTerm(t,id)) place(t,id);
        // lấp đủ min (ưu tiên spec, không project)
        for (const auto& id : topo) {
            if (R.terms[t].totalCredits >= MIN_PER_TERM) break;
            if ((isSpecCoreFn(id) || isSpecElecFn(id) || (!isOtherSpecElec(id) && !isCap(id) && !isIntern(id)))
                && canPlaceTerm(t,id)) place(t,id);
        }
    }
    // 7: Spec Core/Elective còn lại + Spec Project + Internship
    else {
        // PATCH: đặt Internship mặc định ở Kỳ 7 (index 6) trước tiên
        for (const auto& id : topo) if (isIntern(id) && canPlaceTerm(t,id)) place(t,id);

        for (const auto& id : topo) if ((isSpecCoreFn(id) || isSpecElecFn(id)) && canPlaceTerm(t,id)) place(t,id);
        // Project (đưa HẾT project về kỳ 7)
        for (const auto& id : topo) if (isSpecProjFn(id) && canPlaceTerm(t,id)) place(t,id);
        // Internship ở kỳ 7
        // (moved earlier)

        // lấp đủ min (ưu tiên spec)
        for (const auto& id : topo) {
            if (R.terms[t].totalCredits >= MIN_PER_TERM) break;
            if ((isSpecAny(id) || (!isOtherSpecElec(id) && !isCap(id)))
                && canPlaceTerm(t,id)) place(t,id);
        }
    }
}

// ====== K8: đặt Capstone, nếu không đủ chỗ thì dọn K8 -> [0..7] ======
{
    const int t = 7; // Kỳ 8 (0-based)
    // Tìm capId (id hoặc name chứa "CAPSTONE")
    std::string capId = "";
    for (const auto& id : topo) {
        auto it = courses_.find(id);
        if (it == courses_.end()) continue;
        const auto nameU = up(it->second.name);
        if (isCapstoneId(id) || nameU.find("CAPSTONE") != std::string::npos) {
            capId = id; break;
        }
    }
    if (capId.empty()) { R.ok = false; R.message = "Capstone course not found."; return R; }

    auto capCred = courses_.at(capId).credits;

    auto canPlaceCap = [&](int term)->bool {
        if (placed[capId]) return false;
        if (R.terms[term].totalCredits + capCred > MAX_PER_TERM) return false;
        if (!prereqsOkByTerm(R, courses_, capId, term)) return false;
        return true;
    };

    // 1) thử đặt ngay
    if (!canPlaceCap(t)) {
        // 2) nếu thiếu chỗ -> dọn bất kỳ môn thường nào từ K8 về [0..7]
        auto tryMoveOneOut = [&]()->bool {
            // duyệt từ cuối để tránh phá vòng lặp
            for (int i = (int)R.terms[t].courses.size()-1; i >= 0; --i) {
                const auto mvId = R.terms[t].courses[i].id;
                if (mvId == capId) continue;
                // không đẩy Capstone/Intern/Spec Project
                if (isCapstoneId(mvId) || isInternshipId(mvId) || isSpecProjectId(mvId, skey))
                    continue;

                const auto& cmov = courses_.at(mvId);
                // tìm bến đỗ sớm nhất có thể: 0..7 (tránh 8 nếu chính là 8)
                for (int dst = 0; dst <= 7; ++dst) {
                    if (dst == t) continue;
                    // đủ headroom & đủ tiên quyết
                    if (R.terms[dst].totalCredits + cmov.credits > MAX_PER_TERM) continue;
                    if (!prereqsOkByTerm(R, courses_, mvId, dst)) continue;
                    // move
                    R.terms[t].courses.erase(R.terms[t].courses.begin()+i);
                    R.terms[t].totalCredits -= cmov.credits;
                    R.terms[dst].courses.push_back({cmov.id, cmov.name, cmov.credits});
                    R.terms[dst].totalCredits += cmov.credits;
                    return true;
                }
            }
            return false;
        };

        // dọn liên tục cho tới khi đủ headroom hoặc hết cách
        for (int pass = 0; pass < 64 && !canPlaceCap(t); ++pass) {
            if (!tryMoveOneOut()) break;
        }

        if (!canPlaceCap(t)) {
            // nếu vẫn không đặt được -> báo chi tiết
            if (!prereqsOkByTerm(R, courses_, capId, t)) {
                std::vector<std::string> missing;
                for (const auto& pre : courses_.at(capId).prereqs) {
                    if (!courses_.count(pre)) continue;
                    int tp = termIndexOfInPlan(R, pre);
                    if (tp == -1 || tp >= t) missing.push_back(pre);
                }
                std::string msg = "Cannot place Capstone in term 8. Unmet prerequisites: ";
                for (size_t i = 0; i < missing.size(); ++i) {
                    if (i) msg += ", ";
                    msg += missing[i];
                }
                R.ok = false; R.message = msg; return R;
            } else {
                int need = capCred - (MAX_PER_TERM - R.terms[t].totalCredits);
                if (need < 0) need = 0;
                R.ok = false;
                R.message = "Cannot place Capstone in term 8. Not enough credit headroom (need "
                            + std::to_string(need) + " more credits).";
                return R;
            }
        }
    }

    // đặt Capstone
    if (canPlaceCap(t)) {
        place(t, capId);
    } else {
        // safety net (không rơi vào đây nếu trên đã thành công/báo lỗi)
        R.ok = false; R.message = "Cannot place Capstone in term 8."; return R;
    }

    // Phần lấp K8 còn lại của bạn giữ nguyên
}

    // ====== Cân bằng toàn cục: đảm bảo mọi kỳ >= 15 (không phá prereq/Spec>=K6) ======
    auto canPullFromLater = [&](int from, int to, const std::function<bool(const std::string&)>& guard)->bool {
        for (int i = 0; i < (int)R.terms[from].courses.size(); ++i) {
            const auto& id = R.terms[from].courses[i].id;
            if (!guard(id)) continue;
            if (!fits(to, id)) continue;
            if (!prereqsOkByTerm(R, courses_, id, to)) continue;
            // không kéo môn chuyên ngành về trước K6
            if (isSpecAny(id) && to < 5) continue;

            auto moved = R.terms[from].courses[i];
            if (R.terms[from].totalCredits - moved.credits < MIN_PER_TERM) continue;

            R.terms[from].courses.erase(R.terms[from].courses.begin()+i);
            R.terms[from].totalCredits -= moved.credits;
            R.terms[to].courses.push_back(moved);
            R.terms[to].totalCredits += moved.credits;
            return true;
        }
        return false;
    };

    for (int t = 0; t < 8; ++t) {
        while (R.terms[t].totalCredits < MIN_PER_TERM) {
            bool moved = false;
            for (int src = 7; src > t; --src) {
                moved |= canPullFromLater(src, t, [&](const std::string& id){
                    if (isCap(id) || isIntern(id)) return false;
                    if (up(id)=="PROJ215879E") return false;
                    if (t < 5 && isSpecAny(id)) return false; // không đưa spec về trước K6
                    return true;
                });
                if (moved) break;
            }
            if (!moved) break; // không thể cân bằng thêm
        }
    }

    // ======= Validate quotas & unscheduled =======
    // Specialized elective: lấy đúng số môn thực sự có
    int availableSpecElective = 0;
for (const auto& [cid, c] : courses_) {
    if (isSpecElectiveForKey(c, up(skey))) ++availableSpecElective;
}
int needSpecElective = std::min(2, availableSpecElective);

    if (cntITCoreElec < PICK_ITCORE_ELECTIVE) {
        R.ok = false;
        R.message = "Not enough IT Core Elective courses selected (need 4, got " + std::to_string(cntITCoreElec) + ").";
        return R;
    }
    
    if (cntSpecElec < needSpecElective) {
        R.ok = false;
        R.message = "Not enough Specialized Elective courses selected (need " + std::to_string(needSpecElective)
                    + ", got " + std::to_string(cntSpecElec) + ").";
        return R;
    }

    // báo lỗi các môn "áp dụng" mà chưa xếp được
    auto applicable = [&](const std::string& id)->bool {
        const auto& c = courses_.at(id);
        // PATCH: bỏ qua placeholder generic SPC_1/SPC_2
        if (up(id).rfind("SPC_", 0) == 0) return false;
        if (isGeneralTrack(c.track)) return true;
        if (isITCoreTrack(c.track))  return true;
        if (isITCoreElec(id))        return true;
        if (isSpecCore(c, skey))     return true;
        if (isSpecElecFn(id))        return true;
        if (isSpecElectiveForKey(c, up(skey))) return true;
        if (isSpecProjectId(id, skey)) return true;
        if (isCapstoneId(id) || isInternshipId(id)) return true;
        return false;
    };
    for (const auto& [id,_] : courses_) if (!placed.at(id) && applicable(id)) {
        R.ok = false;
        R.message = "Some courses remain unscheduled after 8 terms (e.g., " + id + ").";
        return R;
    }
     // 1) Academic English 1..4: nếu có trong curriculum thì bắt buộc đã xếp
    {
        const std::string aeNames[4] = {
            "Academic English 1", "Academic English 2",
            "Academic English 3", "Academic English 4"
        };
        for (int i = 0; i < 4; ++i) {
            std::string id = findIdByNameContains(courses_, aeNames[i]);
            if (!id.empty()) {
                // nếu môn tồn tại trong data mà chưa được xếp -> lỗi
                if (!placed.count(id) || !placed.at(id)) {
                    R.ok = false;
                    R.message = "Missing required course: " + aeNames[i] + ".";
                    return R;
                }
            }
        }
    }

    // 2) Project chuyên ngành phải nằm ở kỳ 7 (0-based: index = 6)
    {
        std::string projId = "";
        for (const auto& kv : courses_) {
            if (isSpecProjectId(kv.first, skey)) { projId = kv.first; break; }
        }
        if (!projId.empty()) {
            int tProj = termIndexOfInPlan(R, projId);
            if (tProj != 6) {
                R.ok = false;
                R.message = "Specialization Project must be scheduled in term 7.";
                return R;
            }
        }
    }

for (int t = 0; t < 8; ++t) {
    if (R.terms[t].totalCredits > MAX_PER_TERM) {
        R.ok = false;
        R.message = "Term " + std::to_string(t+1) + " exceeds max credits (" +
                    std::to_string(MAX_PER_TERM) + ").";
        return R;
    }
}
// KHÔNG bắt buộc kỳ ≥15 tín chỉ nữa (kỳ trống cũng được)
// PATCH: Bảo đảm vị trí cố định cho IT Project (Kỳ 5) và Internship (Kỳ 7)
{
    auto termIndexOf = [&](const std::string& cid)->int{
        for (int ti = 0; ti < (int)R.terms.size(); ++ti)
            for (auto &c : R.terms[ti].courses)
                if (c.id == cid) return ti;
        return -1;
    };
    auto moveCourse = [&](int from, int to, const std::string& cid)->bool{
        if (from == to || from < 0 || to < 0) return false;
        const auto& cmv = courses_.at(cid);
        if (R.terms[to].totalCredits + cmv.credits > MAX_PER_TERM) return false;
        if (!prereqsOkByTerm(R, courses_, cid, to)) return false;
        for (int i = (int)R.terms[from].courses.size()-1; i >= 0; --i) {
            if (R.terms[from].courses[i].id == cid) {
                auto moved = R.terms[from].courses[i];
                R.terms[from].courses.erase(R.terms[from].courses.begin()+i);
                R.terms[from].totalCredits -= moved.credits;
                R.terms[to].courses.push_back(moved);
                R.terms[to].totalCredits += moved.credits;
                return true;
            }
        }
        return false;
    };

    // Internship -> Kỳ 7 (index 6)
    for (const auto& id : topo) if (isIntern(id)) {
        int ti = termIndexOf(id);
        if (ti != 6 && ti != -1) {
            // cố gắng nhường chỗ kỳ 7 nếu đầy
            if (!moveCourse(ti, 6, id)) {
                // dọn bớt 1 môn thường từ kỳ 7 qua kỳ 8
                for (int i = (int)R.terms[6].courses.size()-1; i >= 0; --i) {
                    const auto mv = R.terms[6].courses[i].id;
                    if (isIntern(mv) || isCap(mv) || isSpecProjectId(mv, skey)) continue;
                    const auto& cmv = courses_.at(mv);
                    if (R.terms[7].totalCredits + cmv.credits > MAX_PER_TERM) continue;
                    if (!prereqsOkByTerm(R, courses_, mv, 7)) continue;
                    R.terms[6].courses.erase(R.terms[6].courses.begin()+i);
                    R.terms[6].totalCredits -= cmv.credits;
                    R.terms[7].courses.push_back({cmv.id, cmv.name, cmv.credits});
                    R.terms[7].totalCredits += cmv.credits;
                    break;
                }
                (void)moveCourse(ti, 6, id);
            }
        }
    }

    // IT Project -> Kỳ 5 (index 4)
    const std::string itp = "PROJ215879E";
    if (courses_.count(itp)) {
        int ti = termIndexOf(itp);
        if (ti != 4 && ti != -1) {
            // nếu K5 đầy: dọn 1 môn thường từ K5 qua K6/K7
            if (R.terms[4].totalCredits + courses_.at(itp).credits > MAX_PER_TERM) {
                bool freed = false;
                for (int i = (int)R.terms[4].courses.size()-1; i >= 0 && !freed; --i) {
                    const auto mv = R.terms[4].courses[i].id;
                    if (mv == itp || isIntern(mv) || isCap(mv) || isSpecProjectId(mv, skey)) continue;
                    const auto& cmv = courses_.at(mv);
                    for (int dst : {5,6}) {
                        if (R.terms[dst].totalCredits + cmv.credits > MAX_PER_TERM) continue;
                        if (!prereqsOkByTerm(R, courses_, mv, dst)) continue;
                        R.terms[4].courses.erase(R.terms[4].courses.begin()+i);
                        R.terms[4].totalCredits -= cmv.credits;
                        R.terms[dst].courses.push_back({cmv.id, cmv.name, cmv.credits});
                        R.terms[dst].totalCredits += cmv.credits;
                        freed = true; break;
                    }
                }
            }
            (void)moveCourse(ti, 4, itp);
        }
    }
}
    R.ok = true; R.message.clear(); return R;
}