#include "PlanConstraints.h"

using namespace std;

void PlanConstraints::validate() const
{
    if (mincredits<=0)
    {
        throw runtime_error("Tín chỉ tối thiểu phải lớn hơn 0");
    }
    if (maxcredits<mincredits)
    {
        throw runtime_error("Tín chỉ tối đa phải lớn hơn hoặc bằng tín chỉ tối thiểu");
    }
    if (numTerms <=0)
    {
        throw runtime_error("Số kì phải lớn hơn 0");
    }
    for (int t : offered_terms)
    {
        if (t<1 || t>numTerms)
        {
            throw runtime_error("Số kì phải nằm trong khoảng từ 1 đến" + to_string(numTerms));
        }
    }
}