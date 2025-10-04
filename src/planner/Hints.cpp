#include "Hints.h"

using namespace std;

namespace {
    HintNote makeHint(const string& msg, const string& key, const string& value) {
        HintNote note;
        note.message = msg;
        note.actions[key] = value;
        return note;
    }
}

vector<HintNote> Hints::analyze(
    int numTerms,
    int maxCredits,
    bool enforceCoreqTogether,
    bool electiveConflict,
    bool preferLightLoad
){
    vector<HintNote> notes;
    if (numTerms < 8) {
        notes.push_back(makeHint(
            "Nên tăng số kỳ học để có thể phân bổ đủ môn.",
            "increase_numTerms_to",
            to_string(numTerms + 1)
        ));
    }

    if (!preferLightLoad && maxCredits < 25) {
        notes.push_back(makeHint(
            "Nên nâng giới hạn tín chỉ tối đa mỗi kỳ để muốn rút ngắn thời gian tốt nghiệp.",
            "increase_maxCredits_to",
            to_string(maxCredits + 3)
        ));
    }

    if (enforceCoreqTogether) {
        notes.push_back(makeHint(
            "Có thể cho phép tách corequisite (coreqTogether).",
            "relax_coreqTogether",
            "true"
        ));
    }

    if (electiveConflict) {
        notes.push_back(makeHint(
            "Cần đổi nhóm môn tự chọn (elective) để tránh trùng lịch.",
            "change_elective_group",
            "1"
        ));
    }

    return notes;
}

