import { BarChart, Bar, XAxis, YAxis, Tooltip, ResponsiveContainer } from 'recharts';
import type { Term } from '../types/plan';


type Props = { terms: Term[]; min?: number; max?: number };


export default function CreditsBarChart({ terms, min, max }: Props) {
const data = terms.map(t => ({ name: `T${t.index}`, credits: t.credits }));
return (
<div className="w-full h-72 bg-white border rounded-xl p-4 shadow-sm">
<ResponsiveContainer width="100%" height="100%">
<BarChart data={data}>
<XAxis dataKey="name" />
<YAxis />
<Tooltip />
<Bar dataKey="credits" />
</BarChart>
</ResponsiveContainer>
{(min || max) && (
<div className="text-xs text-slate-500 mt-2">
{min ? <span>Min/term: {min} &nbsp; </span> : null}
{max ? <span>Max/term: {max}</span> : null}
</div>
)}
</div>
);
}