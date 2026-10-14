// Fake ESP32 for testing without hardware. Emits the same `JSON:` lines
// as the firmware, so the real parsing path is exercised.

export function startDemo(onLine) {
  const startedAt = Date.now();
  let t = 0;
  let timer;

  const beat = () => {
    const seconds = t / 1000;
    const rr = Math.round(900 + 60 * Math.sin(2 * Math.PI * 0.1 * seconds) + (Math.random() - 0.5) * 20);
    t += rr;

    // Ramp coherence up over the first 2 minutes, like a user settling in
    const settled = Math.min(1, (Date.now() - startedAt) / 120000);
    const hasCoherence = seconds >= 30;

    const data = {
      t,
      rr,
      hr: +(60000 / rr).toFixed(1),
      rmssd: seconds >= 27 ? +(45 + 10 * settled).toFixed(1) : 0,
      sdnn: seconds >= 27 ? +(40 + 8 * settled).toFixed(1) : 0,
      pnn50: seconds >= 27 ? +(20 + 10 * settled).toFixed(1) : 0,
      freq: hasCoherence ? 0.1 : 0,
      coh: hasCoherence ? +(0.3 + 0.55 * settled).toFixed(2) : 0,
    };

    onLine(`RR: ${rr} ms | HR: ${data.hr} | Time: ${t} ms`);
    onLine(`JSON:${JSON.stringify(data)}`);

    timer = setTimeout(beat, rr);
  };

  beat();
  return () => clearTimeout(timer);
}
