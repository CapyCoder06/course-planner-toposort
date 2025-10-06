import { useEffect, useState } from 'react';
import type { PlanResult } from '../types/plan';


export function usePlan(url: string) {
const [plan, setPlan] = useState<PlanResult | null>(null);
const [error, setError] = useState<string | null>(null);
const [loading, setLoading] = useState(false);


useEffect(() => {
    let active = true;
    setLoading(true);
    fetch(url)
    .then(r => {
        if (!r.ok) throw new Error(`HTTP ${r.status}`);
        return r.json();
    })
    .then((data: PlanResult) => active && setPlan(data))
    .catch(e => active && setError(String(e)))
    .finally(() => active && setLoading(false));
    return () => {
        active = false;
    };
}, [url]);

return { plan, error, loading };
}