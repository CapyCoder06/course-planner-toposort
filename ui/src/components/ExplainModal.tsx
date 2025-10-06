import type { ExplainEntry } from '../types/plan';


type Props = {
open: boolean;
onClose: () => void;
courseId: string | null;
explain?: ExplainEntry[];
};


export default function ExplainModal({ open, onClose, courseId, explain }: Props) {
if (!open || !courseId) return null;
const entry = explain?.find(e => e.courseId === courseId);
return (
<div className="fixed inset-0 bg-black/40 flex items-center justify-center p-4 z-50">
<div className="bg-white rounded-2xl shadow-xl w-full max-w-lg p-4">
<div className="flex items-center justify-between mb-3">
<h3 className="text-lg font-semibold">Explain: {courseId}</h3>
<button onClick={onClose} className="px-2 py-1 text-sm rounded bg-slate-100 hover:bg-slate-200">Close</button>
</div>
{!entry?.chain?.length ? (
<p className="text-sm text-slate-600">No dependency chain available.</p>
) : (
<div className="text-sm">
<p className="mb-2 text-slate-700">Longest prerequisite chain:</p>
<div className="flex flex-wrap items-center gap-2">
{entry.chain.map((c, i) => (
<div key={i} className="flex items-center gap-2">
<span className="px-2 py-1 rounded bg-slate-50 border"><code>{c}</code></span>
{i < entry.chain!.length - 1 && <span>→</span>}
</div>
))}
</div>
</div>
)}
</div>
</div>
);
}