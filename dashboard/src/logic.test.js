import { test } from 'node:test';
import assert from 'node:assert/strict';

import { parseLine, createLineSplitter } from './serial.js';
import { coherenceStatus } from './coherence.js';

test('parseLine reads JSON lines and ignores the rest', () => {
  assert.deepEqual(parseLine('JSON:{"rr":900,"coh":0.8}'), { rr: 900, coh: 0.8 });
  assert.equal(parseLine('RR: 900 ms | HR: 66.7'), null);
  assert.equal(parseLine('JSON:{broken'), null);
});

test('line splitter joins chunks split mid-line', () => {
  const lines = [];
  const push = createLineSplitter((l) => lines.push(l));

  push('JSON:{"rr":9');
  push('00}\r\nOUTLIER: 4');
  assert.deepEqual(lines, ['JSON:{"rr":900}']);

  push('80\n\n');
  assert.deepEqual(lines, ['JSON:{"rr":900}', 'OUTLIER: 480']);
});

test('coherence needs both a high score and the 0.1 Hz target', () => {
  assert.equal(coherenceStatus(0, 0).level, 'waiting');
  assert.equal(coherenceStatus(0.1, 0.85).level, 'high');
  assert.equal(coherenceStatus(0.1, 0.5).level, 'medium');
  assert.equal(coherenceStatus(0.25, 0.9).label, 'Slow your breathing');
  assert.equal(coherenceStatus(0.05, 0.2).level, 'low');
});
