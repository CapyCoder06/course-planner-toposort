#include "Hints.h"

using namespace std;

// helper nhỏ để tạo ghi chú gợi ý nhanh
namespace {
    HintNote makeHint(const string& msg, const string& key, const string& value) {
        HintNote note;
        note.message = msg;        // nội dung gợi ý
        note.actions[key] = value; // hành động/đề xuất kèm theo
        return note;
    }
}

// phân tích thông số để đưa ra gợi ý chỉnh kế hoạch học tập
vector<HintNote> Hints::analyze(
    int numTerms,              // số kỳ học hiện có
    int maxCredits,            // giới hạn tín chỉ tối đa mỗi kỳ
    bool enforceCoreqTogether, // có bắt buộc học coreq cùng kỳ không
    bool electiveConflict,     // có xung đột nhóm môn tự chọn không
    bool preferLightLoad       // có ưu tiên học nhẹ hay không
){
    vector<HintNote> notes;

    // nếu số kỳ học ít hơn 8 thì gợi ý tăng thêm
    if (numTerms < 8) {
        notes.push_back(makeHint(
            "Nên tăng số kỳ học để có thể phân bổ đủ môn.",
            "increase_numTerms_to",
            to_string(numTerms + 1)
        ));
    }

    // nếu không ưu tiên học nhẹ mà maxCredits còn thấp -> nên tăng thêm
    if (!preferLightLoad && maxCredits < 25) {
        notes.push_back(makeHint(
            "Nên nâng giới hạn tín chỉ tối đa mỗi kỳ để muốn rút ngắn thời gian tốt nghiệp.",
            "increase_maxCredits_to",
            to_string(maxCredits + 3)
        ));
    }

    // nếu đang buộc học coreq cùng nhau thì gợi ý cho phép tách ra
    if (enforceCoreqTogether) {
        notes.push_back(makeHint(
            "Có thể cho phép tách corequisite (coreqTogether).",
            "relax_coreqTogether",
            "true"
        ));
    }

    // nếu có xung đột nhóm tự chọn thì gợi ý đổi nhóm khác
    if (electiveConflict) {
        notes.push_back(makeHint(
            "Cần đổi nhóm môn tự chọn (elective) để tránh trùng lịch.",
            "change_elective_group",
            "try_different_group"
        ));
    }

    return notes;
}
