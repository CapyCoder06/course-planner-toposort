// ui/src/App.tsx
import React, { useEffect, useState } from 'react';
import { RefreshCw, BookOpen, BarChart3, MessageSquare, Upload } from 'lucide-react';
import PlanTable from './components/PlanTable';
import CreditsBarChart from './components/CreditsBarChart';
import NotesPanel from './components/NotesPanel';
import type { PlanResult, PlanNote } from './types/plan';
import { loadPlanMock, type DemoMode } from './adapters/plannerAdapter';

export default function App() {
  const [planData, setPlanData] = useState<(PlanResult & { parsedNotes: PlanNote[] }) | null>(null);
  const [loading, setLoading] = useState(false);
  const [activeTab, setActiveTab] = useState<'table' | 'chart' | 'notes'>('table');
  const [demoMode, setDemoMode] = useState<DemoMode>('feasible');

  const load = async (mode: DemoMode) => {
    setLoading(true);
    try {
      const data = await loadPlanMock(mode);
      setPlanData(data);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => { load(demoMode); }, [demoMode]);

  const tabs = [
    { id: 'table', label: 'Study Plan', icon: BookOpen },
    { id: 'chart', label: 'Credits Chart', icon: BarChart3 },
    { id: 'notes', label: 'Notes & Issues', icon: MessageSquare }
  ] as const;

  return (
    <div className="min-h-screen bg-gradient-to-br from-blue-50 via-white to-purple-50">
      {/* Header */}
      <header className="bg-white shadow-sm border-b">
        <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
          <div className="flex justify-between items-center py-6">
            <div>
              <h1 className="text-3xl font-bold text-gray-900">Course Study Planner</h1>
              <p className="mt-1 text-sm text-gray-600">Topological Sorting for Full-Course Study Planning</p>
            </div>
            <div className="flex items-center space-x-4">
              <div className="flex items-center space-x-2">
                <span className="text-sm text-gray-600">Demo:</span>
                <select
                  value={demoMode}
                  onChange={(e) => setDemoMode(e.target.value as DemoMode)}
                  className="px-3 py-1 border border-gray-300 rounded-md text-sm focus:outline-none focus:ring-2 focus:ring-blue-500"
                >
                  <option value="feasible">Feasible Plan</option>
                  <option value="infeasible">Infeasible Plan</option>
                </select>
              </div>
              <button
                onClick={() => load(demoMode)}
                disabled={loading}
                className="inline-flex items-center px-4 py-2 rounded-md text-sm font-medium text-white bg-blue-600 hover:bg-blue-700 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-blue-500 disabled:opacity-50"
              >
                <RefreshCw className={`w-4 h-4 mr-2 ${loading ? 'animate-spin' : ''}`} />
                Refresh
              </button>
            </div>
          </div>
        </div>
      </header>

      {/* Tabs */}
      <nav className="bg-white border-b">
        <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
          <div className="flex space-x-8">
            {tabs.map((t) => {
              const Icon = t.icon;
              const active = activeTab === t.id;
              return (
                <button
                  key={t.id}
                  onClick={() => setActiveTab(t.id)}
                  className={`flex items-center space-x-2 py-4 px-1 border-b-2 font-medium text-sm transition-colors ${
                    active ? 'border-blue-500 text-blue-600' : 'border-transparent text-gray-500 hover:text-gray-700 hover:border-gray-300'
                  }`}
                >
                  <Icon className="w-5 h-5" />
                  <span>{t.label}</span>
                  {t.id === 'notes' && planData && !planData.feasible && (
                    <span className="inline-flex items-center px-2 py-1 rounded-full text-xs font-medium bg-red-100 text-red-800">!</span>
                  )}
                </button>
              );
            })}
          </div>
        </div>
      </nav>

      {/* Main */}
      <main className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
        {loading ? (
          <div className="flex items-center justify-center py-12">
            <div className="text-center">
              <RefreshCw className="w-8 h-8 text-blue-500 animate-spin mx-auto mb-4" />
              <p className="text-gray-600">Loading plan...</p>
            </div>
          </div>
        ) : planData ? (
          <>
            {activeTab === 'table' && <PlanTable terms={planData.terms} feasible={planData.feasible} />}
            {activeTab === 'chart' && <CreditsBarChart terms={planData.terms} feasible={planData.feasible} maxCreditsPerTerm={15} />}
            {activeTab === 'notes' && <NotesPanel notes={planData.notes} parsedNotes={planData.parsedNotes} feasible={planData.feasible} />}
          </>
        ) : (
          <div className="text-center py-12">
            <Upload className="w-12 h-12 text-gray-400 mx-auto mb-4" />
            <p className="text-gray-600">No plan data available</p>
          </div>
        )}
      </main>
    </div>
  );
}
