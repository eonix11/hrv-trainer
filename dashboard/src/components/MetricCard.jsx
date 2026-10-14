export default function MetricCard({ label, value, unit, digits = 1, hint }) {
  const hasValue = value !== null && value !== undefined;

  return (
    <div className="card">
      <div className="card-label">{label}</div>
      <div className="card-value">
        {hasValue ? value.toFixed(digits) : '—'}
        {hasValue && <span className="unit"> {unit}</span>}
      </div>
      {hint && <div className="card-hint">{hasValue ? hint : 'after 30 beats'}</div>}
    </div>
  );
}
