// ui/src/components/CreditsBarChart.tsx
import React from 'react';
import { ResponsiveContainer, BarChart, Bar, XAxis, YAxis, CartesianGrid, Tooltip } from 'recharts';
import type { Term } from '../types/plan';

export default function CreditsBarChart({
  terms,
  feasible,
  maxCreditsPerTerm = 15
}: {
  terms: Term[];
  feasible: boolean;
  maxCreditsPerTerm?: number;
}) {
  if (!feasible || terms.length === 0) {
    return (
      <div className="bg-white rounded-lg shadow-md p-6">
        <h2 className="text-xl font-bold text-gray-800 mb-4">Credits Distribution</h2>
        <div className="text-center py-8">
          <div className="text-gray-400 text-4xl mb-4">📊</div>
          <p className="text-gray-600">No data to display</p>
        </div>
      </div>
    );
  }

  const chartData = terms.map((t) => ({
    term: `Term ${t.index}`,
    credits: t.credits,
    courses: (t.courses?.length ?? t.courseIds.length),
    isOverloaded: t.credits > maxCreditsPerTerm
  }));

  const totalCredits = terms.reduce((s, t) => s + t.credits, 0);
  const avg = totalCredits / terms.length;
  const max = Math.max(...terms.map((t) => t.credits));
  const min = Math.min(...terms.map((t) => t.credits));

  const CustomTooltip = ({ active, payload, label }: any) => {
    if (active && payload && payload.length) {
      const d = payload[0].payload;
      return (
        <div className="bg-white p-3 border rounded-lg shadow-lg">
          <p className="font-medium text-gray-900">{label}</p>
          <p className="text-sm text-blue-600">Credits: <span className="font-medium">{d.credits}</span></p>
          <p className="text-sm text-gray-600">Courses: <span className="font-medium">{d.courses}</span></p>
          {d.isOverloaded && <p className="text-xs text-red-500 mt-1">⚠️ Over recommended limit</p>}
        </div>
      );
    }
    return null;
  };

  return (
    <div className="bg-white rounded-lg shadow-md p-6">
      <div className="flex items-center justify-between mb-6 text-sm text-gray-600">
        <h2 className="text-xl font-bold text-gray-800">Credits Distribution</h2>
        <div className="space-x-4">
          <span>Max: {max}</span>
          <span>Avg: {avg.toFixed(1)}</span>
          <span>Min: {min}</span>
        </div>
      </div>

      <div className="h-80 w-full">
        <ResponsiveContainer width="100%" height="100%">
          <BarChart data={chartData} margin={{ top: 20, right: 30, left: 20, bottom: 5 }}>
            <CartesianGrid strokeDasharray="3 3" stroke="#f0f0f0" />
            <XAxis dataKey="term" stroke="#666" fontSize={12} tickLine={false} />
            <YAxis stroke="#666" fontSize={12} tickLine={false} axisLine={false} />
            <Tooltip content={<CustomTooltip />} />
            <Bar dataKey="credits" radius={[4, 4, 0, 0]} fill="#3b82f6" />
          </BarChart>
        </ResponsiveContainer>
      </div>

      <div className="mt-4 flex items-center justify-between text-xs text-gray-500">
        <div className="flex items-center space-x-4">
          <div className="flex items-center space-x-1">
            <div className="w-3 h-3 bg-blue-500 rounded" />
            <span>Normal load</span>
          </div>
          <div className="flex items-center space-x-1">
            <div className="w-3 h-3 bg-red-500 rounded" />
            <span>Overloaded (&gt;{maxCreditsPerTerm} credits)</span>
          </div>
        </div>
        <p>Recommended max: {maxCreditsPerTerm} credits/term</p>
      </div>
    </div>
  );
}
