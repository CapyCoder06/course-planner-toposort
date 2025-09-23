// ui/src/components/PlanTable.tsx
import React from 'react';
import type { Term } from '../types/plan';

export default function PlanTable({ terms, feasible }: { terms: Term[]; feasible: boolean }) {
  if (!feasible || terms.length === 0) {
    return (
      <div className="bg-white rounded-lg shadow-md p-6">
        <h2 className="text-xl font-bold text-gray-800 mb-4">Study Plan</h2>
        <div className="text-center py-8">
          <div className="text-red-500 text-6xl mb-4">⚠️</div>
          <p className="text-gray-600 text-lg">{!feasible ? 'Plan is not feasible' : 'No courses in plan'}</p>
        </div>
      </div>
    );
  }

  const totalCredits = terms.reduce((s, t) => s + t.credits, 0);

  return (
    <div className="bg-white rounded-lg shadow-md overflow-hidden">
      <div className="px-6 py-4 bg-gradient-to-r from-blue-600 to-purple-600">
        <h2 className="text-xl font-bold text-white">Study Plan</h2>
        <p className="text-blue-100 text-sm">{terms.length} terms • {totalCredits} total credits</p>
      </div>

      <div className="overflow-x-auto">
        <table className="w-full">
          <thead className="bg-gray-50">
            <tr>
              <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">Term</th>
              <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">Courses</th>
              <th className="px-6 py-3 text-center text-xs font-medium text-gray-500 uppercase">Credits</th>
            </tr>
          </thead>
          <tbody className="bg-white divide-y divide-gray-200">
            {terms.map((term) => (
              <tr key={term.index} className="hover:bg-gray-50 transition-colors">
                <td className="px-6 py-4 whitespace-nowrap">
                  <div className="flex items-center">
                    <div className="h-10 w-10 rounded-full bg-blue-100 flex items-center justify-center">
                      <span className="text-sm font-medium text-blue-600">{term.index}</span>
                    </div>
                    <p className="ml-3 text-sm font-medium text-gray-900">Term {term.index}</p>
                  </div>
                </td>
                <td className="px-6 py-4">
                  <div className="space-y-2">
                    {(term.courses ?? term.courseIds.map((id) => ({ id, name: undefined, credits: undefined as unknown as number }))).map((c: any) => (
                      <div key={c.id ?? c} className="flex items-center justify-between p-3 rounded-lg border border-gray-200 hover:border-blue-300">
                        <div>
                          <p className="text-sm font-medium text-gray-900">{c.id ?? c}</p>
                          {c.name && <p className="text-xs text-gray-600">{c.name}</p>}
                        </div>
                        {c.credits != null && (
                          <span className="inline-flex items-center px-2.5 py-0.5 rounded-full text-xs font-medium bg-blue-100 text-blue-800">
                            {c.credits} credits
                          </span>
                        )}
                      </div>
                    ))}
                  </div>
                </td>
                <td className="px-6 py-4 text-center whitespace-nowrap">
                  <span className="inline-flex items-center px-3 py-1 rounded-full text-sm font-medium bg-green-100 text-green-800">{term.credits}</span>
                </td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>

      <div className="px-6 py-4 bg-gray-50 border-t flex justify-between text-sm">
        <span className="text-gray-600">Total: {terms.length} terms</span>
        <span className="font-medium text-gray-900">Total Credits: {totalCredits}</span>
      </div>
    </div>
  );
}
