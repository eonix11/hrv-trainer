import { useReducer, useRef, useCallback, useEffect } from 'react';

import { isSerialSupported, openSerial, parseLine } from './serial.js';
import { startDemo } from './demo.js';
import { coherenceStatus, breathsPerMinute } from './coherence.js';

import MetricCard from './components/MetricCard.jsx';
import RRChart from './components/RRChart.jsx';
import BreathingGuide from './components/BreathingGuide.jsx';

const CHART_WINDOW_MS = 60000;
const MAX_LOG_LINES = 30;

const initialState = {
  source: 'none',   // 'none' | 'serial' | 'demo'
  latest: null,
  beats: [],
  log: [],
  error: '',
};

function reducer(state, action) {
  switch (action.type) {
    case 'connected':
      return { ...initialState, source: action.source };

    case 'disconnected':
      return { ...state, source: 'none' };

    case 'error':
      return { ...state, error: action.message };

    case 'beat': {
      const beat = action.data;
      // Time went backwards = ESP32 restarted, so start a fresh chart
      const previous = state.beats.length && beat.t < state.beats.at(-1).t ? [] : state.beats;
      const beats = [...previous, { t: beat.t, rr: beat.rr }].filter(
        (b) => b.t >= beat.t - CHART_WINDOW_MS,
      );
      return { ...state, latest: beat, beats };
    }

    case 'log':
      return { ...state, log: [...state.log, action.line].slice(-MAX_LOG_LINES) };

    default:
      return state;
  }
}

export default function App() {
  const [state, dispatch] = useReducer(reducer, initialState);
  const stopRef = useRef(null);

  const handleLine = useCallback((line) => {
    const data = parseLine(line);
    if (data) dispatch({ type: 'beat', data });
    else dispatch({ type: 'log', line });
  }, []);

  const stop = useCallback(async () => {
    const stopFn = stopRef.current;
    stopRef.current = null;
    if (stopFn) await stopFn();
    dispatch({ type: 'disconnected' });
  }, []);

  const connectSerial = async () => {
    await stop();
    try {
      stopRef.current = await openSerial(handleLine, () => dispatch({ type: 'disconnected' }));
      dispatch({ type: 'connected', source: 'serial' });
    } catch (err) {
      // User closed the port picker = not an error worth showing
      if (err.name !== 'NotFoundError') dispatch({ type: 'error', message: err.message });
    }
  };

  const runDemo = async () => {
    await stop();
    stopRef.current = startDemo(handleLine);
    dispatch({ type: 'connected', source: 'demo' });
  };

  useEffect(() => () => stopRef.current?.(), []);

  const d = state.latest;
  const status = coherenceStatus(d?.freq ?? 0, d?.coh ?? 0);
  const hasHrv = d && d.rmssd > 0;

  return (
    <div className="app">
      <header className="header">
        <h1>HRV Trainer</h1>
        <div className="controls">
          <span className={`dot dot-${state.source}`} />
          <span className="source">
            {state.source === 'serial' ? 'ESP32 connected' : state.source === 'demo' ? 'Demo data' : 'Not connected'}
          </span>
          {state.source === 'none' ? (
            <>
              <button onClick={connectSerial} disabled={!isSerialSupported()}>Connect ESP32</button>
              <button className="secondary" onClick={runDemo}>Demo</button>
            </>
          ) : (
            <button className="secondary" onClick={stop}>Disconnect</button>
          )}
        </div>
      </header>

      {!isSerialSupported() && (
        <p className="notice">Web Serial needs Chrome or Edge. Use Demo to try the dashboard here.</p>
      )}
      {state.error && <p className="notice error">{state.error}</p>}

      <main className="grid">
        <section className={`panel coherence level-${status.level}`}>
          <h2>Coherence</h2>
          <div className="big">{d?.freq ? `${Math.round(d.coh * 100)}%` : '—'}</div>
          <div className="status">{status.label}</div>
          <div className="sub">
            {d?.freq
              ? `${d.freq.toFixed(3)} Hz · ${breathsPerMinute(d.freq).toFixed(1)} breaths/min`
              : 'Needs 30 s of clean beats'}
          </div>
        </section>

        <section className="panel">
          <BreathingGuide level={status.level} />
        </section>

        <section className="panel wide">
          <h2>RR interval · last 60 s</h2>
          <RRChart beats={state.beats} windowMs={CHART_WINDOW_MS} />
        </section>

        <section className="cards wide">
          <MetricCard label="Heart rate" value={d?.hr} unit="BPM" digits={0} />
          <MetricCard label="RR" value={d?.rr} unit="ms" digits={0} />
          <MetricCard label="RMSSD" value={hasHrv ? d.rmssd : null} unit="ms" hint="beat-to-beat change" />
          <MetricCard label="SDNN" value={hasHrv ? d.sdnn : null} unit="ms" hint="overall spread" />
          <MetricCard label="pNN50" value={hasHrv ? d.pnn50 : null} unit="%" hint="pairs > 50 ms apart" />
        </section>

        <section className="panel wide">
          <h2>Serial log</h2>
          <pre className="log">{state.log.join('\n') || 'Waiting for data…'}</pre>
        </section>
      </main>
    </div>
  );
}
