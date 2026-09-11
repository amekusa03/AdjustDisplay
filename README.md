# AdjustDisplay - Visual Display Calibration Tool

[日本語版はこちら (README.jp.md)](README.jp.md)

AdjustDisplay is a Qt C++ desktop application designed for precise visual display calibration (brightness, contrast, gamma, and color balance) on Ubuntu and Linux environments.

## Features

1. **Multi-Display Detection & Screen Identification**
   - Automatically detects connected displays (resolution, refresh rate, DPI, color depth, geometry).
   - "Identify" feature overlays large numbered banners on target monitors.

2. **High-Precision Test Pattern Rendering Engine**
   - **Black Level / Brightness**: 0% to 5% low-luminance stepped bars with blinking test blocks to set the shadow threshold.
   - **White Level / Contrast**: 95% to 100% highlight bars and RGB channel highlights to prevent highlight clipping.
   - **Gamma 2.2**: 1px alternating black/white raster stripes against luminance reference patches for standard sRGB tone curve alignment.
   - **Grayscale & Color Balance**: 32-step grayscale gradient bar, continuous 8-bit ramp, and primary RGB linearity inspection.
   - **Sharpness & Focus**: 1px horizontal/vertical lines and checkerboards to eliminate ringing, halos, and moire artifacts.
   - **Color Uniformity & Dead Pixel Check**: Fullscreen pure colors (White, Gray 50%, Black, Red, Green, Blue, Cyan, Magenta, Yellow) to detect vignetting, tinting, and pixel defects.
   - **Screen Ratio & Overscan**: 1:1 pixel mapping, dot-by-dot verification, and 1px outer frame clipping inspection.

3. **Step-by-Step Guided Wizard**
   - Streamlined calibration workflow with actionable monitor OSD adjustment guidance.
   - Intuitive keyboard shortcuts (`Space`/`Enter`/`Right` for Next, `Backspace`/`Left` for Previous, `Esc` to Exit, `F` to toggle Fullscreen, `H` to toggle HUD guide, `C` to cycle colors).

4. **Hardware & Software Integration (DDC/CI & XRandR)**
   - Direct monitor hardware parameter control (Brightness & Contrast) via `ddcutil` where supported.
   - Automatic fallback to XRandR software calibration or manual monitor OSD mode.

5. **Bilingual Support (English / Japanese)**
   - Dynamic real-time switching between English and Japanese from the UI header.
   - Automatically detects system locale and remembers user language preference.

## Build Instructions

### Prerequisites
```bash
sudo apt update
sudo apt install build-essential cmake ninja-build qtbase5-dev libqt5widgets5
```
*(Installing `ddcutil` via `sudo apt install ddcutil` is recommended for direct hardware control)*

### Compile & Run
```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Launch application
./build/adjust-display
```
