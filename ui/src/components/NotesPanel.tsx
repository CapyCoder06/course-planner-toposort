import type { PlanResult } from '../types/plan';


type Props = { plan: PlanResult };


export default function NotesPanel({ plan }: Props) {
if (!plan.notes || plan.notes.length === 0) return null;
return (
<div className="bg-white border rounded-xl p-4 shadow-sm">
<h3 className="text-base font-semibold mb-2">Notes & Hints</h3>
<ul className="list-disc pl-5 space-y-1">
{plan.notes.map((n, i) => (
<li key={i} className="text-sm">
{typeof n === 'string' ? n : JSON.stringify(n)}
</li>
))}
</ul>
</div>
);
}