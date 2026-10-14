# HRV Coherence Trainer

A real-time Heart Rate Variability (HRV) biofeedback system built around an ESP32 and a MAX30100 pulse oximeter sensor.

The firmware detects heartbeats, turns them into RR intervals, filters noisy beats, calculates HRV metrics, and finds the dominant breathing-related rhythm in the heart signal. The long-term goal is live coherence feedback for paced-breathing training.

> **Status:** Active development
> **Hardware:** ESP32 + MAX30100
> **Firmware:** C++ / Arduino
> **Primary goal:** Real-time HRV coherence training

---

## How it works

```text
MAX30100
   │
   ▼
Optical pulse signal
   │
   ▼
Beat detection            (MAX30100 library callback)
   │
   ▼
RR intervals              (time between two beats, in ms)
   │
   ▼
Artifact filtering        (range check + sudden-change check)
   │
   ├──► RRBuffer  (30 beats)  ──► HRVEngine        ──► HR, RMSSD, SDNN, pNN50
   │
   └──► TimeSeries (120 beats) ──► CoherenceEngine ──► dominant frequency
                                                          │
                                                          ▼
                                                 Real-time feedback (planned)
```

### 1. Beat detection
The MAX30100 library calls `onBeatDetected()` each time it finds a heartbeat. The RR interval is the time since the previous beat, measured with `millis()`.

### 2. Artifact filtering
Optical sensors produce false beats when the finger moves. Two filters reject them:

| Filter | Rule | Default |
|---|---|---|
| Range | RR must be between `MIN_RR` and `MAX_RR` | 700–1200 ms (about 50–86 BPM) |
| Sudden change | RR may not differ from the last accepted RR by more than `MAX_CHANGE_PERCENT` | 15 % |

Rejected beats are printed as `OUTLIER:` or `REJECTED:` on the serial monitor.

> The default range fits a resting adult. If your resting heart rate is above ~85 BPM, lower `MIN_RR` in `hrv_trainer.ino`.

### 3. HRV metrics (`HRVEngine`)
Every 30 accepted beats, the firmware calculates:

| Metric | Meaning |
|---|---|
| Average HR | Mean heart rate in BPM |
| RMSSD | Root mean square of differences between neighbouring RR intervals. Short-term, vagal (parasympathetic) activity. |
| SDNN | Standard deviation of all RR intervals. Overall variability. |
| pNN50 | Percent of neighbouring RR pairs that differ by more than 50 ms. |

The 30-beat buffer is then cleared and a new window starts.

### 4. Coherence analysis (`CoherenceEngine`)
RR intervals arrive at uneven times (one per beat). To do frequency analysis, the engine:

1. Resamples the RR series to a regular **4 Hz** grid (one sample every 250 ms) with linear interpolation.
2. Removes the mean (DC offset).
3. Runs a simple DFT from **0.04 Hz to 0.20 Hz** in 0.01 Hz steps.
4. Reports the frequency with the highest power as the **dominant frequency**.

During slow paced breathing (about 6 breaths per minute) the dominant frequency should sit near **0.1 Hz**.

The coherence score itself is not implemented yet (it always returns 0).

---

## Hardware

| Part | Notes |
|---|---|
| ESP32 dev board | Any common ESP32 board with I2C on GPIO 21/22 |
| MAX30100 module | Pulse oximeter / heart-rate sensor, I2C |

### Wiring

| MAX30100 | ESP32 |
|---|---|
| VIN | 3.3V |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 22 |

Some cheap MAX30100 boards have pull-up resistors tied to 1.8 V, which can break I2C. If `MAX30100 init FAILED` appears, check the pull-ups on your module.

---

## Getting started

### Requirements
- [Arduino IDE](https://www.arduino.cc/en/software) or [arduino-cli](https://arduino.github.io/arduino-cli/)
- ESP32 board package (`esp32` by Espressif Systems)
- **MAX30100lib** by OXullo Intersecans (provides `MAX30100_PulseOximeter.h`)

### Arduino IDE
1. Open `hrv_trainer.ino`. The folder name must stay `hrv_trainer`.
2. Select your ESP32 board and port.
3. Upload.
4. Open the Serial Monitor at **115200 baud**.

### arduino-cli
```bash
arduino-cli core install esp32:esp32
arduino-cli lib install "MAX30100lib"

arduino-cli compile --fqbn esp32:esp32:esp32 .
arduino-cli upload  --fqbn esp32:esp32:esp32 -p <PORT> .
arduino-cli monitor -p <PORT> -c baudrate=115200
```

### Usage
1. Place a fingertip on the sensor and keep it still.
2. Wait for steady `RR:` lines.
3. After 30 accepted beats, an `HRV RESULTS` block is printed.
4. After 20+ beats (and at least 10 s of data), the dominant frequency is printed on each beat.

Example output (numbers are only for illustration):
```text
RR: 912 ms | HR: 65.8 | Time: 48213 ms
...
========== HRV RESULTS ==========
Valid beats: 30
Average HR: 66.1 BPM
RMSSD: 38.4 ms
SDNN: 45.2 ms
pNN50: 17.2 %
=================================
Dominant frequency: 0.10 Hz
```

---

## Project structure

| File | Purpose |
|---|---|
| `hrv_trainer.ino` | Entry point: sensor setup, beat callback, filters, serial output |
| `RRBuffer.h/.cpp` | Fixed 30-slot buffer of accepted RR intervals for HRV windows |
| `TimeSeries.h/.cpp` | Fixed 120-slot list of beat events (timestamp, RR, HR) |
| `HRVEngine.h/.cpp` | Time-domain HRV metrics: HR, RMSSD, SDNN, pNN50 |
| `CoherenceEngine.h/.cpp` | 4 Hz resampling + DFT to find the dominant frequency |

---

## Known limitations

- `TimeSeries` is never cleared. After 120 beats new beats are dropped, so the coherence result stops updating.
- `CoherenceEngine` holds at most 240 samples (60 s at 4 Hz). Longer series are cut to the first 60 s.
- The coherence score is not implemented yet.
- All analysis and serial printing run inside the beat callback, which can delay `pox.update()`.

## Roadmap

- [x] Beat detection and RR extraction
- [x] Artifact filtering
- [x] Time-domain HRV metrics (RMSSD, SDNN, pNN50)
- [x] Resampling and dominant-frequency detection
- [ ] Sliding window for `TimeSeries` (rolling analysis)
- [ ] Coherence score (peak power vs. total power)
- [ ] Breathing pacer and visual/audio feedback
- [ ] Display or BLE/app output

---

## Disclaimer

This is a hobby and learning project. It is not a medical device and must not be used for diagnosis or treatment.
