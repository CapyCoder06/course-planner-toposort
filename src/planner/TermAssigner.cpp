#include "TermAssigner.h"
#include <stdexcept>
#include <algorithm>
using namespace std;

PlanResult assignTermsGreedy(const CourseGraph& g,
                             const TopoResult& topo,
                             const vector<int>& earliestTermByIdx,
                             const vector<int>& creditsByIdx,
                             const PlanConstraints& constraints) {
    if (!topo.success) {
        throw runtime_error("TermAssigner: topo failed (cycle present)");
    }
    const int V = g.V;
    PlanResult res;
    res.termOfIdx.assign(V, 0);

    if ((int)earliestTermByIdx.size() != V || (int)creditsByIdx.size() != V) {
        throw runtime_error("TermAssigner: size mismatch");
    }
    const int T = constraints.numTerms; // must be > 0
    if (T <= 0) {
        throw runtime_error("TermAssigner: constraints.numTerms must be > 0");
    }

    vector<int> termCredits(T + 1, 0); // 1..T
    int currentTerm = 1;
    // ---- tie-break: sort candidates by earliestTerm (asc), then out-degree (desc), stable on topo ----
    vector<int> outdeg(g.V, 0);
    for (int u = 0; u < g.V; ++u) outdeg[u] = (int)g.adj[u].size();

    vector<int> order = topo.order;
    stable_sort(order.begin(), order.end(), [&](int a, int b) {
        if (earliestTermByIdx[a] != earliestTermByIdx[b])
            return earliestTermByIdx[a] < earliestTermByIdx[b];
        return outdeg[a] > outdeg[b];
    });
    for (int u : topo.order) {
        int credits = creditsByIdx[u];
        if (credits < 0) {
            throw runtime_error("TermAssigner: negative credits");
        }

        // earliest term enforced by prerequisites
        int t = earliestTermByIdx[u];
        if (t < 1) t = 1;

        // also respect currentTerm progression
        if (t < currentTerm) t = currentTerm;

        // if this course has specific offered_terms in constraints (optional), you could adjust t to the next offered term >= t here.

        // try to place in a feasible term respecting max credits
        while (t <= T && termCredits[t] + credits > constraints.maxCreditsPerTerm) {
            ++t;
        }

        if (t > T) {
            res.ok = false;
            res.notes.push_back("Infeasible: out of terms while respecting quotas. Consider increasing numTerms or maxCreditsPerTerm.");
            // leave unassigned (0) for this and remaining; or break early
            break;
        }

        res.termOfIdx[u] = t;
        termCredits[t] += credits;
        // advance currentTerm if we filled this term close to quota (simple heuristic: if cannot fit any 1-credit further, you might choose to advance)
        if (termCredits[t] >= constraints.maxCreditsPerTerm) {
            currentTerm = t + 1;
        } else {
            currentTerm = t; // stay on the same term for next items
        }
    }
    return res;
}
