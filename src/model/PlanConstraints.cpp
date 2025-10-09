#include "model/PlanConstraints.h"
#include <stdexcept>
#include <string>

using namespace std;

void PlanConstraints::validate() const {
    if (minCreditsPerTerm <= 0)
    throw invalid_argument("Số tín chỉ tối thiểu (" + to_string(minCreditsPerTerm) + ") phải > 0");

    if (maxCreditsPerTerm < minCreditsPerTerm)
    throw invalid_argument("Tối đa (" + to_string(maxCreditsPerTerm) + 
                           ") nhỏ hơn tối thiểu (" + to_string(minCreditsPerTerm) + ")");

    if (numTerms <= 0)
    throw invalid_argument("Số học kỳ (" + to_string(numTerms) + ") phải > 0");

    for (int t : offered_terms) {
        if (t < 1 || t > numTerms) {
            throw invalid_argument("Học kỳ " + to_string(t) + 
                                " không hợp lệ, cho phép [1.." + to_string(numTerms) + "]");
        }
    }
}