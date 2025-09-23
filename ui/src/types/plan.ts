// ui/src/types/plan.ts
export interface Course {
  id: string;
  name: string;
  credits: number;
}

export interface Term {
  index: number;
  courseIds: string[];
  courses?: Course[];
  credits: number;
}

export interface PlanResult {
  feasible: boolean;
  terms: Term[];
  notes: string[];
  totalCredits?: number;
  totalTerms?: number;
}

export type PlanNoteType = 'error' | 'warning' | 'hint' | 'info';

export interface PlanNote {
  type: PlanNoteType;
  message: string;
  action?: string;
}
