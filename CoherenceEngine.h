#ifndef COHERENCEENGINE_H
#define COHERENCEENGINE_H

#include "TimeSeries.h"

class CoherenceEngine {
private:
  float dominantFrequency;
  float coherenceScore;
  static const int SAMPLE_RATE = 4;
  static const int MAX_SAMPLES = 240;

  float samples[MAX_SAMPLES];
  int sampleCount;

  bool buildSignal(const TimeSeries& series);

public:
  CoherenceEngine();

  void calculate(const TimeSeries& series);

  float getDominantFrequency() const;
  float getCoherenceScore() const;
};

#endif