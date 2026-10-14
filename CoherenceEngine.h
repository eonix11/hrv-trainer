#ifndef COHERENCEENGINE_H
#define COHERENCEENGINE_H

#include "TimeSeries.h"

class CoherenceEngine {
private:
  float dominantFrequency;
  float coherenceScore;

  static const int SAMPLE_RATE = 4;              // Hz, 250 ms grid
  static const int MAX_SAMPLES = 256;            // 64 s window
  static const unsigned long MIN_DURATION = 30000; // ms, a few slow cycles

  // Spectrum: 0.040 .. 0.400 Hz in 0.005 Hz steps
  static const int NUM_BINS = 73;
  static const int PEAK_SEARCH_BINS = 45;        // 0.040 .. 0.260 Hz
  static const int PEAK_HALF_WIDTH = 3;          // +/- 0.015 Hz around peak

  float samples[MAX_SAMPLES];
  int sampleCount;

  float spectrum[NUM_BINS];

  bool buildSignal(const TimeSeries& series);
  void detrend();
  void applyWindow();
  float binPower(float frequency) const;
  static float binFrequency(int bin);

public:
  CoherenceEngine();

  bool calculate(const TimeSeries& series);

  float getDominantFrequency() const;
  float getCoherenceScore() const;
};

#endif
