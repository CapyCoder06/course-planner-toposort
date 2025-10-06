import type { Term } from '../types/plan';


type Props = { terms: Term[]; onSelectCourse?: (id: string) => void };


export default function PlanTable({ terms, onSelectCourse }: Props) {
return (
<div className="overflow-x-auto">
<table className="min-w-full border border-slate-200 bg-white shadow-sm rounded-xl">
<thead className="bg-slate-100">
<tr>
<th className="px-4 py-2 text-left">Term</th>
<th className="px-4 py-2 text-left">Courses</th>
<th className="px-4 py-2 text-left">Credits</th>
</tr>
</thead>
<tbody>
{terms.map(t => (
<tr key={t.index} className="border-t">
<td className="px-4 py-2 font-semibold">{t.index}</td>
<td className="px-4 py-2">
<div className="flex flex-wrap gap-2">
{t.courses.map(c => (
<button
key={c.id}
onClick={() => onSelectCourse?.(c.id)}
className="px-2 py-1 rounded-lg bg-slate-50 border hover:bg-slate-100 text-sm"
title={c.name || c.id}
>
<code>{c.id}</code>{c.name ? ` — ${c.name}` : ''}
</button>
))}
</div>
</td>
<td className="px-4 py-2">{t.credits}</td>
</tr>
))}
</tbody>
</table>
</div>
);
}