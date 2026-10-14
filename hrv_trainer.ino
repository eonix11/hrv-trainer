#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

#include "RRBuffer.h"
#include "TimeSeries.h"
#include "HRVEngine.h"
#include "CoherenceEngine.h"

PulseOximeter pox;

RRBuffer rrBuffer;
TimeSeries timeSeries;
HRVEngine hrv;
CoherenceEngine coherence;

#define MIN_RR 700
#define MAX_RR 1200
#define MAX_CHANGE_PERCENT 15

unsigned long lastBeat = 0;
unsigned long totalBeats = 0;
bool beatReady = false;


// ========================================
// HEARTBEAT
// ========================================

void onBeatDetected()
{
  unsigned long now = millis();

  if (lastBeat == 0)
  {
    lastBeat = now;
    return;
  }

  int rr = now - lastBeat;


  // Filter 1: RR range
  if (rr < MIN_RR || rr > MAX_RR)
  {
    Serial.print("OUTLIER: ");
    Serial.println(rr);

    lastBeat = now;
    return;
  }


  // Filter 2: sudden change
  if (rrBuffer.size() > 0)
  {
    int previousRR = rrBuffer.get(rrBuffer.size() - 1);

    float change =
      abs(rr - previousRR) * 100.0 / previousRR;

    if (change > MAX_CHANGE_PERCENT)
    {
      Serial.print("REJECTED: ");
      Serial.println(rr);

      lastBeat = now;
      return;
    }
  }

  // Store data
  rrBuffer.add(rr);

  timeSeries.add(now, rr);

  // Print
  Serial.print("RR: ");
  Serial.print(rr);

  Serial.print(" ms | HR: ");
  Serial.print(60000.0 / rr, 1);

  Serial.print(" | Time: ");
  Serial.print(now);

  Serial.println(" ms");

  totalBeats++;
  beatReady = true;   // heavy work runs in loop(), not here

  lastBeat = now;
}


// ========================================
// PROCESSING (called from loop)
// ========================================

void processBeat()
{
  BeatEvent latest =
    timeSeries.get(timeSeries.size() - 1);


  // Show TimeSeries every 10 beats
  if (totalBeats % 10 == 0)
  {
    Serial.print("TimeSeries events: ");
    Serial.println(timeSeries.size());

    Serial.print("Latest timestamp: ");
    Serial.println(latest.timestamp);

    Serial.print("Latest RR: ");
    Serial.println(latest.rr);
  }


  // Calculate HRV after 30 beats
  if (rrBuffer.isFull())
  {
    hrv.calculate(rrBuffer);

    Serial.println();
    Serial.println("========== HRV RESULTS ==========");

    Serial.print("Valid beats: ");
    Serial.println(rrBuffer.size());

    Serial.print("Average HR: ");
    Serial.print(hrv.getAverageHR(), 1);
    Serial.println(" BPM");

    Serial.print("RMSSD: ");
    Serial.print(hrv.getRMSSD(), 1);
    Serial.println(" ms");

    Serial.print("SDNN: ");
    Serial.print(hrv.getSDNN(), 1);
    Serial.println(" ms");

    Serial.print("pNN50: ");
    Serial.print(hrv.getPNN50(), 1);
    Serial.println(" %");

    Serial.println("=================================");
    Serial.println();


    rrBuffer.clear();
  }


  // Coherence on the latest 64 s
  if (coherence.calculate(timeSeries))
  {
    Serial.print("Dominant frequency: ");
    Serial.print(coherence.getDominantFrequency(), 3);
    Serial.print(" Hz | Coherence: ");
    Serial.println(coherence.getCoherenceScore(), 2);
  }


  // Machine-readable line for the dashboard
  Serial.printf(
    "JSON:{\"t\":%lu,\"rr\":%d,\"hr\":%.1f,\"rmssd\":%.1f,\"sdnn\":%.1f,"
    "\"pnn50\":%.1f,\"freq\":%.3f,\"coh\":%.2f}\n",
    latest.timestamp,
    latest.rr,
    latest.hr,
    hrv.getRMSSD(),
    hrv.getSDNN(),
    hrv.getPNN50(),
    coherence.getDominantFrequency(),
    coherence.getCoherenceScore());
}


// ========================================
// SETUP
// ========================================

void setup()
{
  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("Initializing MAX30100...");

  Wire.begin(21, 22);


  if (!pox.begin())
  {
    Serial.println("MAX30100 init FAILED");

    while (1);
  }


  Serial.println("MAX30100 READY");
  Serial.println("Place finger on sensor.");
  Serial.println("Keep finger still.");
  Serial.println();


  pox.setOnBeatDetectedCallback(onBeatDetected);
}


// ========================================
// LOOP
// ========================================

void loop()
{
  pox.update();

  if (beatReady)
  {
    beatReady = false;
    processBeat();
  }
}
