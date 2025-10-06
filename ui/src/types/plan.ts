export type PlanCourseRef = { id: string; name?: string; credits?: number };
export type Term = { index: number; courses: PlanCourseRef[]; credits: number };
export type ExplainEntry = { courseId: string; chain?: string[] };
export type PlanResult = {
    feasible: boolean;
    terms: Term[];
    totalCredits?: number;
    notes?: Array<string | Record<string, unknown>>;
    explain?: ExplainEntry[];
};