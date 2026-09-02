# HRV Coherence Trainer

A real-time Heart Rate Variability (HRV) biofeedback system built around an ESP32 and MAX30100 pulse oximeter sensor.

The project captures heartbeat timing, extracts RR intervals, filters noisy measurements, calculates HRV metrics, and is being extended toward real-time breathing-heart synchronization and coherence feedback.

> **Status:** Active development  
> **Hardware:** ESP32 + MAX30100  
> **Firmware:** C++ / Arduino  
> **Primary Goal:** Real-time HRV coherence training

---

## Overview

The core idea is to transform optical pulse measurements into a structured physiological signal that can be analyzed in real time.

```text
MAX30100
   │
   ▼
Optical pulse signal
   │
   ▼
Beat detection
   │
   ▼
RR intervals
   │
   ▼
Artifact filtering
   │
   ▼
Time-series data
   │
   ├──► HRV metrics
   │
   └──► Coherence analysis
            │
            ▼
      Real-time feedback
