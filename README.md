# ESP32 Bluetooth Polysynth & Sequencer

A versatile, real-time polyphonic synthesizer built for the **ESP32**, capable of streaming high-quality audio directly to Bluetooth speakers or headphones using the **A2DP profile**.

##  Features

* **Polyphonic Synthesis**: Play up to 8 notes simultaneously with independent envelope control.
* **Wavetable Engine**: Includes 4 classic waveforms: Sine, Triangle, Sawtooth, and Square.
* **3-Track Sequencer**: Running Bass, Pad, and Lead patterns simultaneously with automated patch switching.
* **Real-time FX & Modulation**:
    * **Pitch Shifting**: Dynamic octave control via potentiometer.
    * **LFO Modulation**: Adjustable speed LFO controlling a resonant low-pass filter.
    * **Reverb/Echo Engine**: Integrated circular delay buffer for spatial depth.
* **ADSR Envelopes**: Smooth Attack and Release for every triggered note to prevent audio clicking.

---

##  Hardware Requirements

### Pin Mapping (ESP32)

| Component | Pin | Function |
| :--- | :--- | :--- |
| **Note Buttons (x8)** | 4, 16, 17, 22, 18, 19, 21, 23 | Note Triggers (C4 to C5) |
| **Sequencer Button** | 14 | Toggle Auto-Playback |
| **Pitch Pot** | 35 | Octave Selector (0.5x, 1x, 2x) |
| **Waveform Pot** | 34 | Waveform Selector (Sine, Tri, Saw, Square) |
| **LFO Speed Pot** | 33 | Filter Modulation Speed |
| **Reverb Pot** | 32 | Delay/Reverb Mix Level |

---

##  Software & Libraries

This project relies on the following libraries:
* [ESP32-A2DP](https://github.com/pschatzmann/ESP32-A2DP) by Phil Schatzmann.
* Standard Arduino ESP32 core.

### Configuration
1. Change the `BLUETOOTH_NAME` variable in the code to match your target speaker/headphones:
   ```cpp
   const char* BLUETOOTH_NAME = "Your Device Name";
Important: Ensure your ESP32 partition scheme supports Bluetooth (Select "Huge APP" or "No OTA" in the Arduino IDE Tools menu).

Technical Overview
The Audio Engine
The synth uses a Wavetable Synthesis approach. Instead of calculating expensive math functions in real-time for every sample, it reads pre-computed values from a 512-sample array.

The audio callback get_audio_data processes frames at 44.1kHz:

Mixing: It sums 8 independent oscillators.

Filtering: Applies a simple IIR Low-Pass Filter with a cutoff frequency modulated by a sine-wave LFO.

Spatial FX: The signal is fed into a Circular Delay Buffer (10,000 samples) to create a reverb/echo effect before clipping protection.

## Installation
1. Clone this repository.

2. Install the ESP32-A2DP library in your Arduino IDE.

3. Select ESP32 Dev Module as your board.

4. Set Partition Scheme to Huge APP.

5. Upload the code and open the Serial Monitor (115200 baud).

6. Once "Bluetooth ON" appears, pair your device and start playing!
