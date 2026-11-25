# Tomato Ripeness Sorting System

An embedded quality-control prototype that detects tomatoes on a ramp, classifies them as **ripe** or **unripe** by color using computer vision, and sorts them with a servo motor. It runs on a Raspberry Pi 3 with a USB webcam.

Final project for the **Embedded Operating Systems** course (FGA0103), Electronics Engineering, University of Brasília (UnB).

## Overview

Consistent fruit color is a key quality indicator in tomato-sauce production. This project explores a low-cost alternative to industrial optical sorters: a Raspberry Pi that inspects each fruit in real time and separates the ones that are not ready for processing.

## How it works

1. **Detection:** a reflective infrared sensor detects when a tomato reaches the inspection point.
2. **Capture:** the webcam takes a 640×480 frame of the fruit.
3. **Classification:** the image is converted from BGR to HSV with OpenCV, and two color masks are applied: red (ripe) and orange/yellow/green (unripe). If red pixels make up **90% or more** of the masked area, the tomato is classified as ripe.
4. **Actuation:** two threads run in parallel:
   - the **LEDs** signal the result for 3 seconds (green = ripe, red = unripe);
   - the **servo motor** turns to about +45° (ripe) or -45° (unripe) to route the fruit, then returns to neutral.
5. The system goes back to waiting for the next tomato.

The algorithm was first prototyped in Python (`tomate_v1.py`) and then ported to C++ (`tomate_v2.cc` to `tomate_v8.cc`) to drive the GPIOs and run the tasks concurrently.

## Hardware

| Component | Raspberry Pi connection |
| --- | --- |
| Raspberry Pi 3 + SD card | — |
| USB webcam | USB port |
| Reflective IR sensor (OUT) | GPIO17 (pin 11) |
| Servo motor, standard 38 g (PWM) | GPIO18 (pin 12) |
| Green LED + 110 Ω resistor | GPIO5 (pin 29) |
| Red LED + 220 Ω resistor | GPIO6 (pin 31) |

The IR sensor and the servo are powered from the Pi's 5 V and GND pins. The full schematic is in [`Esquematicos/Esquematico_V3.png`](Esquematicos/Esquematico_V3.png).

## Repository structure

```
Codigos/            Source code (tomate_v1.py prototype, tomate_v2–v8.cc C++ versions; v8 is the latest)
Esquematicos/       Circuit schematics (V1 to V3)
Pontos_de_controle/ Technical reports for each project milestone (in Portuguese)
```

## Running locally

### Requirements

- Raspberry Pi with Raspberry Pi OS and the hardware wired as above
- `g++`, OpenCV 4 (C++ development files) and the WiringPi library

```bash
sudo apt update
sudo apt install -y g++ pkg-config libopencv-dev
```

WiringPi is no longer shipped with recent Raspberry Pi OS releases. If `apt install wiringpi` fails, build it from the community-maintained repository:

```bash
git clone https://github.com/WiringPi/WiringPi.git
cd WiringPi && ./build
```

### Build and run

```bash
cd Codigos
g++ -std=c++11 tomate_v8.cc -o tomate $(pkg-config --cflags --libs opencv4) -lwiringPi -lpthread
sudo ./tomate
```

The program loops continuously, prints whether a fruit was detected and its classification, and drives the LEDs and servo. Stop it with `Ctrl+C`.

### Testing the classifier without hardware

The Python prototype classifies a single image file and needs only OpenCV:

```bash
pip install opencv-python
cd Codigos
python3 tomate_v1.py
```

The script reads an image named `maduro1.jpeg` from the same folder. Add a tomato photo with that name, or edit the file name in the script.

## Limitations

- Classification relies on fixed HSV thresholds, so it works best with good, uniform lighting and evenly colored fruit.
- The servo is driven by software PWM (`softPwm`), which can cause small position jitter.

  ## Video

## Author

- João Pedro Bandeira Raposo
