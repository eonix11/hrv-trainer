import { useEffect, useRef, useState } from 'react';

// 5 s in + 5 s out = 10 s per breath = 6 breaths/min = 0.1 Hz
const INHALE_MS = 5000;
const EXHALE_MS = 5000;
const CYCLE_MS = INHALE_MS + EXHALE_MS;

export default function BreathingGuide({ level }) {
  const [running, setRunning] = useState(false);
  const [phase, setPhase] = useState('in');
  const circleRef = useRef(null);

  useEffect(() => {
    if (!running) return;

    const start = performance.now();
    let frame;

    // Animate with requestAnimationFrame + a ref, so React only
    // re-renders when the phase label changes, not 60 times a second.
    const tick = (now) => {
      const elapsed = (now - start) % CYCLE_MS;
      const inhaling = elapsed < INHALE_MS;
      const progress = inhaling ? elapsed / INHALE_MS : 1 - (elapsed - INHALE_MS) / EXHALE_MS;
      const eased = 0.5 - 0.5 * Math.cos(Math.PI * progress);

      if (circleRef.current) {
        circleRef.current.style.transform = `scale(${0.55 + 0.45 * eased})`;
      }
      setPhase(inhaling ? 'in' : 'out');   // no re-render if unchanged

      frame = requestAnimationFrame(tick);
    };

    frame = requestAnimationFrame(tick);
    return () => cancelAnimationFrame(frame);
  }, [running]);

  return (
    <div className="breathing">
      <h2>Breathing guide</h2>
      <div className="breath-stage">
        <div ref={circleRef} className={`breath-circle level-${level}`} />
        <div className="breath-label">
          {running ? (phase === 'in' ? 'Breathe in' : 'Breathe out') : '6 breaths / min'}
        </div>
      </div>
      <button className="secondary" onClick={() => setRunning((r) => !r)}>
        {running ? 'Stop' : 'Start'}
      </button>
    </div>
  );
}
