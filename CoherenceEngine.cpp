#include <Arduino.h>
#include "CoherenceEngine.h"

CoherenceEngine::CoherenceEngine() {
  dominantFrequency = 0;
  coherenceScore = 0;
}

void CoherenceEngine::calculate(const TimeSeries& series) {
  dominantFrequency = 0;
  coherenceScore = 0;

  if (series.size() < 20) {
    return;
  }

  if (!buildSignal(series)) {
    Serial.print("Not enough data for signal");
    return;
  }

  float average = 0;

  for (int i = 0; i < sampleCount; i++) {
    average += samples[i];
  }

  average /= sampleCount;

  for (int i = 0; i < sampleCount; i++) {
    samples[i] -= average;
  }

  Serial.print("Coherence input events: ");
  Serial.println(series.size());

  Serial.print("Regular samples: ");
  Serial.println(sampleCount);

  float strongestPower = 0;

  for (float frequency = 0.04; frequency <= 0.20; frequency += 0.01) {
    float real = 0;
    float imag = 0;

    for (int i = 0; i < sampleCount; i++) {
      float angle = 2.0 * PI * frequency * i / SAMPLE_RATE;

      real += samples[i] * cos(angle);
      imag += samples[i] * sin(angle);
    }

    float power = real * real + imag * imag;
    if (power > strongestPower) {
      strongestPower = power;
      dominantFrequency = frequency;
    }
  }

  Serial.print("Dominant frequency: ");
  Serial.print(dominantFrequency, 2);
  Serial.println(" Hz");
}

float CoherenceEngine::getDominantFrequency() const {
  return dominantFrequency;
}

float CoherenceEngine::getCoherenceScore() const {
  return coherenceScore;
}

bool CoherenceEngine::buildSignal(const TimeSeries& series) {
  sampleCount = 0;

  if (series.size() < 2)
    return false;

  unsigned long startTime = series.get(0).timestamp;

  unsigned long endTime =
    series.get(series.size() - 1).timestamp;

  unsigned long duration =
    endTime - startTime;

  if (duration < 10000)
    return false;

  unsigned long interval =
    1000 / SAMPLE_RATE;

  unsigned long currentTime =
    startTime;

  int eventIndex = 0;

  while (currentTime <= endTime && sampleCount < MAX_SAMPLES) {
    while (eventIndex < series.size() - 2 && series.get(eventIndex + 1).timestamp < currentTime) {
      eventIndex++;
    }

    BeatEvent a = series.get(eventIndex);
    BeatEvent b = series.get(eventIndex + 1);

    float timeRange =
      b.timestamp - a.timestamp;

    float value;

    if (timeRange > 0) {
      float position =
        (currentTime - a.timestamp) / timeRange;

      value =
        a.rr + position * (b.rr - a.rr);
    } else {
      value = a.rr;
    }

    samples[sampleCount] = value;

    sampleCount++;

    currentTime += interval;
  }

  return sampleCount >= 20;
}