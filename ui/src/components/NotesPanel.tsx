// ui/src/components/NotesPanel.tsx
import React, { useState } from 'react';
import { AlertCircle, CheckCircle, Info, Lightbulb, X, ChevronDown, ChevronRight } from 'lucide-react';
import type { PlanNote } from '../types/plan';

export default function NotesPanel({
  notes,
  parsedNotes,
  feasible
}: {
  notes: string[];
  parsedNotes: PlanNote[];
  feasible: boolean;
}) {
  const [isExpanded, setIsExpanded] = useState(!feasible);
  const [dismissed, setDismissed] = useState<Set<number>>(new Set());

  const getIcon = (t: PlanNote['type']) =>
    t === 'error' ? <AlertCircle className="w-5 h-5 text-red-500" /> :
    t === 'warning' ? <AlertCircle className="w-5 h-5 text-yellow-500" /> :
    t === 'hint' ? <Lightbulb className="w-5 h-5 text-blue-500" /> :
    <Info className="w-5 h-5 text-gray-500" />;

  const bg = (t: PlanNote['type']) =>
    t === 'error' ? 'bg-red-50 border-red-200' :
    t === 'warning' ? 'bg-yellow-50 border-yellow-200' :
    t === 'hint' ? 'bg-blue-50 border-blue-200' :
    'bg-gray-50 border-gray-200';

  const dismiss = (i: number) => setDismissed(new Set([...dismissed, i]));
  const visible = parsedNotes.filter((_, i) => !dismissed.has(i));
  const hasImportant = parsedNotes.some(n => n.type === 'error' || n.type === 'warning');

  if (notes.length === 0) {
    return (
      <div className="bg-white rounded-lg shadow-md p-6">
        <div className="flex items-center space-x-2">
          <CheckCircle className="w-5 h-5 text-green-500" />
          <h2 className="text-xl font-bold text-gray-800">Plan Status</h2>
        </div>
        <p className="mt-2 text-gray-600">No issues detected. Your study plan is ready!</p>
      </div>
    );
  }

  return (
    <div className="bg-white rounded-lg shadow-md overflow-hidden">
      <div
        className={`px-6 py-4 cursor-pointer transition-colors ${!feasible ? 'bg-red-100 hover:bg-red-50' : 'bg-green-100 hover:bg-green-50'}`}
        onClick={() => setIsExpanded(!isExpanded)}
      >
        <div className="flex items-center justify-between">
          <div className="flex items-center space-x-2">
            {!feasible ? <AlertCircle className="w-5 h-5 text-red-600" /> : <CheckCircle className="w-5 h-5 text-green-600" />}
            <h2 className="text-xl font-bold text-gray-800">{!feasible ? 'Plan Issues' : 'Plan Notes'}</h2>
            <span className="inline-flex items-center px-2.5 py-0.5 rounded-full text-xs font-medium bg-white text-gray-800">
              {visible.length}
            </span>
          </div>
          {isExpanded ? <ChevronDown className="w-5 h-5 text-gray-400" /> : <ChevronRight className="w-5 h-5 text-gray-400" />}
        </div>
        {hasImportant && !isExpanded && (
          <p className="mt-1 text-sm text-red-600">
            Click to view {parsedNotes.filter(n => n.type === 'error' || n.type === 'warning').length} important issue(s)
          </p>
        )}
      </div>

      {isExpanded && (
        <div className="px-6 py-4 space-y-4 max-h-96 overflow-y-auto">
          {visible.length === 0 ? (
            <div className="text-center py-8">
              <CheckCircle className="w-12 h-12 text-green-500 mx-auto mb-2" />
              <p className="text-gray-600">All issues have been dismissed</p>
            </div>
          ) : (
            visible.map((note, idx) => (
              <div key={idx} className={`border rounded-lg p-4 ${bg(note.type)}`}>
                <div className="flex items-start justify-between">
                  <div className="flex items-start space-x-3 flex-1">
                    <div className="flex-shrink-0 mt-0.5">{getIcon(note.type)}</div>
                    <div className="flex-1">
                      <p className="text-sm font-medium text-gray-900 capitalize">{note.type}</p>
                      <p className="mt-1 text-sm text-gray-700">{note.message}</p>
                      {note.action && (
                        <div className="mt-2 p-2 bg-white bg-opacity-50 rounded border border-white border-opacity-50">
                          <p className="text-xs text-gray-600">
                            <strong>Suggested action:</strong> {note.action}
                          </p>
                        </div>
                      )}
                    </div>
                  </div>
                  {note.type !== 'error' && (
                    <button
                      onClick={() => dismiss(idx)}
                      className="flex-shrink-0 ml-2 p-1 hover:bg-white hover:bg-opacity-50 rounded transition-colors"
                      title="Dismiss this note"
                    >
                      <X className="w-4 h-4 text-gray-400 hover:text-gray-600" />
                    </button>
                  )}
                </div>
              </div>
            ))
          )}

          {!feasible && visible.some(n => n.type === 'hint') && (
            <div className="mt-6 p-4 bg-blue-50 border border-blue-200 rounded-lg">
              <h4 className="text-sm font-medium text-blue-900 mb-2">Quick Fixes</h4>
              <ul className="text-xs text-blue-800 space-y-1">
                {visible.filter(n => n.type === 'hint').map((n, i) => (
                  <li key={i} className="flex items-start space-x-1"><span>•</span><span>{n.action || n.message}</span></li>
                ))}
              </ul>
            </div>
          )}
        </div>
      )}
    </div>
  );
}
