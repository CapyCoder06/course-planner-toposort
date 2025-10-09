#include "PlannerService.h"
#include <string>
#include <unordered_set>
#include <algorithm>

static std::string up(std::string v) {
    for (auto &ch : v) ch = (char)toupper((unsigned char)ch);
    return v;
}

// ---------------- Track helpers ----------------
bool PlannerService::isGeneralTrack(std::string t) {
    t = up(t);
    return t == "GENERAL" || t == "SUPPLEMENT";
}

bool PlannerService::isITCoreTrack(std::string t) {
    t = up(t);
    return t == "ITCORE" || t == "IT CORE";
}

bool PlannerService::isITCoreElectiveTrack(std::string t) {
    if (t.empty()) return false;
    t = up(t);
    return t == "IT CORE ELECTIVE" || t == "ITCOREELECTIVE" || t == "IT ELECTIVE";
}

std::string PlannerService::specKey(Specialization s) {
    switch (s) {
        case Specialization::SE:  return "SE";
        case Specialization::NNS: return "NNS";
        case Specialization::IS:  return "IS";
        case Specialization::AI:  return "AI";
        default: return "";
    }
}

bool PlannerService::isSpecCore(const Course& c, const std::string& skey) {
    return up(c.track) == ("SPEC:" + up(skey));
}

bool PlannerService::isSpecElective(const Course& c, const std::string& skey) {
    return up(c.track) == up(skey);
}

bool PlannerService::isSpecProjectId(const std::string& id, const std::string& skey) {
    auto key = up(skey);
    auto idU = up(id);
    if (key == "SE")  return idU == "POSE431479E";
    if (key == "NNS") return idU == "POCN431280E";
    if (key == "IS")  return idU == "POIS431184E";
    return false;
}

bool PlannerService::isCapstoneId(const std::string& id) {
    return up(id) == "GRPR471979E";
}
