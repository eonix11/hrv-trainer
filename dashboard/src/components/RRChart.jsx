// Plain SVG line chart: x = time (last windowMs), y = RR in ms.

const WIDTH = 600;
const HEIGHT = 200;
const PAD = { left: 44, right: 12, top: 12, bottom: 24 };

export default function RRChart({ beats, windowMs }) {
  if (beats.length < 2) {
    return <div className="chart-empty">Waiting for beats…</div>;
  }

  const tEnd = beats.at(-1).t;
  const tStart = tEnd - windowMs;

  const rrs = beats.map((b) => b.rr);
  const min = Math.min(...rrs) - 20;
  const max = Math.max(...rrs) + 20;

  const x = (t) => PAD.left + ((t - tStart) / windowMs) * (WIDTH - PAD.left - PAD.right);
  const y = (rr) => PAD.top + (1 - (rr - min) / (max - min)) * (HEIGHT - PAD.top - PAD.bottom);

  const points = beats.map((b) => `${x(b.t).toFixed(1)},${y(b.rr).toFixed(1)}`).join(' ');
  const ticks = [0, 10, 20, 30, 40, 50, 60];

  return (
    <svg className="chart" viewBox={`0 0 ${WIDTH} ${HEIGHT}`} role="img" aria-label="RR interval over the last 60 seconds">
      {[min, (min + max) / 2, max].map((v) => (
        <g key={v}>
          <line className="grid-line" x1={PAD.left} x2={WIDTH - PAD.right} y1={y(v)} y2={y(v)} />
          <text className="axis" x={PAD.left - 6} y={y(v) + 4} textAnchor="end">{Math.round(v)}</text>
        </g>
      ))}
      {ticks.map((s) => (
        <text key={s} className="axis" x={x(tStart + s * 1000)} y={HEIGHT - 6} textAnchor="middle">
          {s === 60 ? 'now' : `-${60 - s}s`}
        </text>
      ))}
      <polyline className="rr-line" points={points} />
      <circle className="rr-dot" cx={x(tEnd)} cy={y(beats.at(-1).rr)} r="4" />
    </svg>
  );
}
