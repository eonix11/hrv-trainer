#include <Arduino.h>
#include "CoherenceEngine.h"

CoherenceEngine::CoherenceEngine() {
  dominantFrequency = 0;
  coherenceScore = 0;
  sampleCount = 0;
}

bool CoherenceEngine::calculate(const TimeSeries& series) {
  dominantFrequency = 0;
  coherenceScore = 0;

  if (!buildSignal(series)) {
    return false;
  }

  detrend();
  applyWindow();

  // Power spectrum + strongest peak in the breathing band
  float totalPower = 0;
  int peakBin = 0;

  for (int bin = 0; bin < NUM_BINS; bin++) {
    spectrum[bin] = binPower(binFrequency(bin));
    totalPower += spectrum[bin];

    if (bin < PEAK_SEARCH_BINS && spectrum[bin] > spectrum[peakBin]) {
      peakBin = bin;
    }
  }

  dominantFrequency = binFrequency(peakBin);

  // Coherence: share of total power that sits near the peak
  float peakPower = 0;
  int from = max(0, peakBin - PEAK_HALF_WIDTH);
  int to = min(NUM_BINS - 1, peakBin + PEAK_HALF_WIDTH);

  for (int bin = from; bin <= to; bin++) {
    peakPower += spectrum[bin];
  }

  if (totalPower > 0) {
    coherenceScore = peakPower / totalPower;
  }

  return true;
}

float CoherenceEngine::getDominantFrequency() const {
  return dominantFrequency;
}

float CoherenceEngine::getCoherenceScore() const {
  return coherenceScore;
}

float CoherenceEngine::binFrequency(int bin) {
  return 0.04 + bin * 0.005;
}

// Resample the irregular RR series onto a regular 4 Hz grid, using the
// most recent MAX_SAMPLES / SAMPLE_RATE seconds of data.
bool CoherenceEngine::buildSignal(const TimeSeries& series) {
  sampleCount = 0;

  if (series.size() < 2)
    return false;

  unsigned long interval = 1000 / SAMPLE_RATE;

  unsigned long firstTime = series.get(0).timestamp;
  unsigned long endTime = series.get(series.size() - 1).timestamp;

  if (endTime - firstTime < MIN_DURATION)
    return false;

  unsigned long windowLength = (MAX_SAMPLES - 1) * interval;
  unsigned long startTime = firstTime;

  if (endTime - firstTime > windowLength)
    startTime = endTime - windowLength;

  unsigned long currentTime = startTime;
  int eventIndex = 0;

  while (currentTime <= endTime && sampleCount < MAX_SAMPLES) {
    while (eventIndex < series.size() - 2 && series.get(eventIndex + 1).timestamp < currentTime) {
      eventIndex++;
    }

    BeatEvent a = series.get(eventIndex);
    BeatEvent b = series.get(eventIndex + 1);

    float timeRange = b.timestamp - a.timestamp;
    float value;

    if (timeRange > 0) {
      float position = (currentTime - a.timestamp) / timeRange;
      value = a.rr + position * (b.rr - a.rr);
    } else {
      value = a.rr;
    }

    samples[sampleCount] = value;
    sampleCount++;

    currentTime += interval;
  }

  return sampleCount >= MIN_DURATION / interval;
}

// Remove mean and slow linear drift (least-squares line fit).
// Without this, HR drift shows up as fake power at the lowest bin.
void CoherenceEngine::detrend() {
  float n = sampleCount;
  float sumX = 0, sumY = 0, sumXY = 0, sumXX = 0;

  for (int i = 0; i < sampleCount; i++) {
    sumX += i;
    sumY += samples[i];
    sumXY += i * samples[i];
    sumXX += (float)i * i;
  }

  float slope = (n * sumXY - sumX * sumY) / (n * sumXX - sumX * sumX);
  float offset = (sumY - slope * sumX) / n;

  for (int i = 0; i < sampleCount; i++) {
    samples[i] -= offset + slope * i;
  }
}

// Hann window: fade both ends to zero to reduce spectral leakage.
void CoherenceEngine::applyWindow() {
  for (int i = 0; i < sampleCount; i++) {
    float w = 0.5 * (1.0 - cos(2.0 * PI * i / (sampleCount - 1)));
    samples[i] *= w;
  }
}

// Goertzel: power at one frequency, 1 cos() per bin instead of 2 per sample.
float CoherenceEngine::binPower(float frequency) const {
  float coeff = 2.0 * cos(2.0 * PI * frequency / SAMPLE_RATE);
  float s1 = 0;
  float s2 = 0;

  for (int i = 0; i < sampleCount; i++) {
    float s0 = samples[i] + coeff * s1 - s2;
    s2 = s1;
    s1 = s0;
  }

  return s1 * s1 + s2 * s2 - coeff * s1 * s2;
}
