#pragma once

#include <QString>
#include <QColor>
#include <QVector>

enum class PatternType {
    BlackLevel,       // Brightness: 0% - 5% low luminance step blocks
    WhiteLevel,       // Contrast: 95% - 100% highlight clipping blocks
    Gamma22,          // Gamma 2.2 calibration: 1px black/white alternating raster vs gray patches
    GrayRamp,         // 32-step & smooth grayscale ramp + RGB balance
    ColorUniformity,  // Fullscreen pure colors (White, Gray, Red, Green, Blue, etc.)
    Sharpness,        // Fine 1px grid / moire / concentric circles
    GeometryFocus     // Crosshatch & circle geometry
};

enum class UniformityColor {
    White,
    Gray50,
    Black,
    Red,
    Green,
    Blue,
    Cyan,
    Magenta,
    Yellow
};

struct WizardStepInfo {
    PatternType patternType;
    QString title;
    QString subtitle;
    QString instruction;
    QString osdTip;
    UniformityColor defaultUniformityColor = UniformityColor::White;
};
