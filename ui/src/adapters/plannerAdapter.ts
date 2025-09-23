// ui/src/adapters/plannerAdapter.ts
import feasibleData from '../mocks/plan.sample.json';
import infeasibleData from '../mocks/plan.infeasible.json';
import type { PlanNote, PlanResult } from '../types/plan';

const parsePlanNotes = (notes: string[]): PlanNote[] =>
  notes.map((note) => {
    if (note.startsWith('ERROR:')) {
      return { type: 'error', message: note.replace('ERROR: ', ''), action: 'Fix the circular dependency or missing prerequisites' };
    } else if (note.startsWith('HINT:')) {
      return { type: 'hint', message: note.replace('HINT: ', ''), action: 'Consider adjusting the constraints' };
    } else if (note.startsWith('WARNING:')) {
      return { type: 'warning', message: note.replace('WARNING: ', '') };
    }
    return { type: 'info', message: note };
  });

export type DemoMode = 'feasible' | 'infeasible';

export async function loadPlanMock(mode: DemoMode): Promise<PlanResult & { parsedNotes: PlanNote[] }> {
  // mô phỏng delay I/O
  await new Promise((r) => setTimeout(r, 300));
  const data = mode === 'feasible' ? (feasibleData as PlanResult) : (infeasibleData as PlanResult);
  return { ...data, parsedNotes: parsePlanNotes(data.notes) };
}
