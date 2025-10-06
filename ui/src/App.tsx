// ui/src/App.tsx
import { useEffect, useState } from "react";

// Các kiểu tối thiểu để compile (khớp với plan.json demo)
type PlanCourseRef = { id: string; name?: string; credits?: number };
type Term = { index: number; courses: PlanCourseRef[]; credits: number };
type PlanResult = {
  feasible: boolean;
  terms: Term[];
  totalCredits?: number;
  notes?: Array<string | Record<string, unknown>>;
  explain?: { courseId: string; chain?: string[] }[];
};

export default function App() {
  const [src, setSrc] = useState("/sample.plan.json");
  const [plan, setPlan] = useState<PlanResult | null>(null);
  const [loading, setLoading] = useState(false);
  const [err, setErr] = useState<string | null>(null);

  useEffect(() => {
    let active = true;
    setLoading(true);
    fetch(src)
      .then((r) => {
        if (!r.ok) throw new Error(`HTTP ${r.status}`);
        return r.json();
      })
      .then((data: PlanResult) => {
        if (active) setPlan(data);
      })
      .catch((e) => active && setErr(String(e)))
      .finally(() => active && setLoading(false));
    return () => {
      active = false;
    };
  }, [src]);

  return (
    <div className="max-w-5xl mx-auto p-4 space-y-4">
      <header className="flex items-center justify-between">
        <h1 className="text-2xl font-bold">Study Planner — Minimal UI</h1>
        <div className="flex items-center gap-2">
          <input
            type="text"
            value={src}
            onChange={(e) => setSrc(e.target.value)}
            className="border rounded px-3 py-2 text-sm w-72"
            placeholder="/sample.plan.json"
          />
          <button
            onClick={() => window.location.reload()}
            className="border rounded px-3 py-2 text-sm bg-white"
          >
            Reload
          </button>
        </div>
      </header>

      {loading && <div>Loading plan…</div>}
      {err && <div className="text-red-600">Error: {err}</div>}

      {plan && (
        <div className="space-y-3">
          <div className="text-sm text-slate-600">
            Feasible: <b>{String(plan.feasible)}</b>
          </div>
          <div className="overflow-x-auto">
            <table className="min-w-full border border-slate-200 bg-white rounded">
              <thead className="bg-slate-100">
                <tr>
                  <th className="px-3 py-2 text-left">Term</th>
                  <th className="px-3 py-2 text-left">Courses</th>
                  <th className="px-3 py-2 text-left">Credits</th>
                </tr>
              </thead>
              <tbody>
                {plan.terms.map((t) => (
                  <tr key={t.index} className="border-t">
                    <td className="px-3 py-2 font-semibold">{t.index}</td>
                    <td className="px-3 py-2">
                      <div className="flex flex-wrap gap-2">
                        {t.courses.map((c) => (
                          <span
                            key={c.id}
                            className="px-2 py-1 text-sm border rounded bg-slate-50"
                            title={c.name || c.id}
                          >
                            <code>{c.id}</code>
                            {c.name ? ` — ${c.name}` : ""}
                          </span>
                        ))}
                      </div>
                    </td>
                    <td className="px-3 py-2">{t.credits}</td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>

          {plan.notes?.length ? (
            <div className="bg-white border rounded p-3">
              <div className="font-semibold mb-2">Notes</div>
              <ul className="list-disc pl-5 text-sm space-y-1">
                {plan.notes.map((n, i) => (
                  <li key={i}>{typeof n === "string" ? n : JSON.stringify(n)}</li>
                ))}
              </ul>
            </div>
          ) : null}
        </div>
      )}
    </div>
  );
}
