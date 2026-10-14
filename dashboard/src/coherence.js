// Turns the firmware's raw numbers into something a user can act on.

export const TARGET_MIN_HZ = 0.08;   // ~4.8 breaths/min
export const TARGET_MAX_HZ = 0.12;   // ~7.2 breaths/min
export const HIGH_COHERENCE = 0.6;
export const MEDIUM_COHERENCE = 0.4;

// A high score alone is not enough: normal breathing (~0.25 Hz) can also
// give a clean wave. Coherent = high score AND near the 0.1 Hz target.
export function coherenceStatus(freq, coh) {
  if (!freq) return { level: 'waiting', label: 'Collecting data…' };

  const onTarget = freq >= TARGET_MIN_HZ && freq <= TARGET_MAX_HZ;

  if (onTarget && coh >= HIGH_COHERENCE) return { level: 'high', label: 'Coherent' };
  if (onTarget && coh >= MEDIUM_COHERENCE) return { level: 'medium', label: 'Getting there' };
  if (!onTarget && coh >= MEDIUM_COHERENCE) return { level: 'low', label: 'Slow your breathing' };

  return { level: 'low', label: 'Low coherence' };
}

export function breathsPerMinute(freq) {
  return freq * 60;
}
