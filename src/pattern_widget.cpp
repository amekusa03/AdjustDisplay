#include "pattern_widget.h"
#include "i18n.h"
#include <QPainter>
#include <QPainterPath>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QScreen>
#include <QtMath>

PatternWidget::PatternWidget(QWidget *parent) : QWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    m_hudFadeTimer.setSingleShot(true);
    m_hudFadeTimer.setInterval(5000);
    connect(&m_hudFadeTimer, &QTimer::timeout, this, [this]() {
        if (m_hudVisible) {
            m_hudVisible = false;
            update();
        }
    });

    m_blinkTimer.setInterval(600);
    connect(&m_blinkTimer, &QTimer::timeout, this, [this]() {
        m_blinkPhase = (m_blinkPhase + 1) % 2;
        if (m_patternType == PatternType::BlackLevel || m_patternType == PatternType::WhiteLevel) {
            update();
        }
    });
    m_blinkTimer.start();

    connect(I18n::instance(), &I18n::languageChanged, this, [this]() {
        update();
    });
}

PatternWidget::~PatternWidget() = default;

void PatternWidget::setPatternType(PatternType type) {
    m_patternType = type;
    update();
}

void PatternWidget::setUniformityColor(UniformityColor color) {
    m_uniformityColor = color;
    emit uniformityColorChanged(color);
    update();
}

void PatternWidget::cycleUniformityColor() {
    int next = (static_cast<int>(m_uniformityColor) + 1) % 9;
    setUniformityColor(static_cast<UniformityColor>(next));
}

void PatternWidget::setGuideInfo(const QString &title, const QString &instruction, const QString &osdTip) {
    m_guideTitle = title;
    m_guideInstruction = instruction;
    m_guideOsdTip = osdTip;
    m_hudVisible = true;
    m_hudFadeTimer.start();
    update();
}

void PatternWidget::setHudVisible(bool visible) {
    m_hudVisible = visible;
    update();
}

void PatternWidget::setStepNavigation(int currentStep, int totalSteps) {
    m_currentStep = currentStep;
    m_totalSteps = totalSteps;
    update();
}

void PatternWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    switch (m_patternType) {
    case PatternType::BlackLevel:
        drawBlackLevelPattern(painter);
        break;
    case PatternType::WhiteLevel:
        drawWhiteLevelPattern(painter);
        break;
    case PatternType::Gamma22:
        drawGamma22Pattern(painter);
        break;
    case PatternType::GrayRamp:
        drawGrayRampPattern(painter);
        break;
    case PatternType::ColorUniformity:
        drawColorUniformityPattern(painter);
        break;
    case PatternType::Sharpness:
        drawSharpnessPattern(painter);
        break;
    case PatternType::GeometryFocus:
        drawGeometryPattern(painter);
        break;
    }

    if (m_hudVisible) {
        drawHudOverlay(painter);
    }
}

void PatternWidget::drawBlackLevelPattern(QPainter &p) {
    auto *i18n = I18n::instance();
    // Pure black background
    p.fillRect(rect(), QColor(0, 0, 0));

    int w = width();
    int h = height();

    // 1px pure white outer boundary border to verify geometry/clipping
    p.setPen(QColor(40, 40, 40));
    p.drawRect(0, 0, w - 1, h - 1);

    // Title / instructions in dark gray so it doesn't disturb eye dark-adaptation
    p.setPen(QColor(160, 160, 160));
    QFont font("SansSerif", 14, QFont::Bold);
    p.setFont(font);
    p.drawText(QRect(0, 40, w, 30), Qt::AlignCenter, i18n->t("pat_black_title"));

    p.setFont(QFont("SansSerif", 11));
    p.setPen(QColor(120, 120, 120));
    p.drawText(QRect(0, 75, w, 25), Qt::AlignCenter, i18n->t("pat_black_desc"));

    // Stepped low-luminance bars: 0%, 0.5%, 1%, 2%, 3%, 4%, 5%, 8%, 10%
    const QVector<QPair<double, QString>> steps = {
        {0.0, "0%\n(RGB 0)"},
        {0.5, "0.5%\n(RGB 1)"},
        {1.0, "1.0%\n(RGB 3)"},
        {2.0, "2.0%\n(RGB 5)"},
        {3.0, "3.0%\n(RGB 8)"},
        {4.0, "4.0%\n(RGB 10)"},
        {5.0, "5.0%\n(RGB 13)"},
        {8.0, "8.0%\n(RGB 20)"},
        {10.0, "10.0%\n(RGB 26)"}
    };

    int n = steps.size();
    int barWidth = qMin(110, (w - 100) / n);
    int barHeight = qMin(240, h / 3);
    int totalBarsWidth = n * barWidth;
    int startX = (w - totalBarsWidth) / 2;
    int startY = h / 2 - barHeight / 2 - 20;

    for (int i = 0; i < n; ++i) {
        int x = startX + i * barWidth;
        int rgbVal = qRound((steps[i].first / 100.0) * 255.0);
        if (steps[i].first == 0.5) rgbVal = 1;

        QRect barRect(x + 2, startY, barWidth - 4, barHeight);
        p.fillRect(barRect, QColor(rgbVal, rgbVal, rgbVal));

        // Thin separator frame
        p.setPen(QColor(30, 30, 30));
        p.drawRect(barRect);

        // Label below
        p.setPen(QColor(150, 150, 150));
        p.setFont(QFont("SansSerif", 9));
        p.drawText(QRect(x, startY + barHeight + 8, barWidth, 36), Qt::AlignCenter, steps[i].second);
    }

    // Blinking ultra-low luminance verification blocks (RGB 2 and RGB 4)
    int blinkY = startY + barHeight + 60;
    int boxSize = 70;
    int blinkX1 = w / 2 - boxSize - 30;
    int blinkX2 = w / 2 + 30;

    if (m_blinkPhase == 0) {
        p.fillRect(QRect(blinkX1, blinkY, boxSize, boxSize), QColor(2, 2, 2));
        p.fillRect(QRect(blinkX2, blinkY, boxSize, boxSize), QColor(4, 4, 4));
    } else {
        p.fillRect(QRect(blinkX1, blinkY, boxSize, boxSize), QColor(0, 0, 0));
        p.fillRect(QRect(blinkX2, blinkY, boxSize, boxSize), QColor(0, 0, 0));
    }
    p.setPen(QColor(25, 25, 25));
    p.drawRect(blinkX1, blinkY, boxSize, boxSize);
    p.drawRect(blinkX2, blinkY, boxSize, boxSize);

    p.setPen(QColor(90, 90, 90));
    p.drawText(QRect(0, blinkY + boxSize + 10, w, 20), Qt::AlignCenter, i18n->t("pat_black_blink"));
}

void PatternWidget::drawWhiteLevelPattern(QPainter &p) {
    auto *i18n = I18n::instance();
    // Pure white background (RGB 255, 255, 255)
    p.fillRect(rect(), QColor(255, 255, 255));

    int w = width();
    int h = height();

    p.setPen(QColor(50, 50, 50));
    QFont font("SansSerif", 14, QFont::Bold);
    p.setFont(font);
    p.drawText(QRect(0, 40, w, 30), Qt::AlignCenter, i18n->t("pat_white_title"));

    p.setFont(QFont("SansSerif", 11));
    p.setPen(QColor(90, 90, 90));
    p.drawText(QRect(0, 75, w, 25), Qt::AlignCenter, i18n->t("pat_white_desc"));

    // Stepped high-luminance highlight bars
    const QVector<QPair<int, QString>> steps = {
        {230, "90%\n(230)"},
        {240, "94%\n(240)"},
        {245, "96%\n(245)"},
        {248, "97%\n(248)"},
        {250, "98%\n(250)"},
        {252, "99%\n(252)"},
        {253, "99.2%\n(253)"},
        {254, "99.6%\n(254)"},
        {255, "100%\n(255)"}
    };

    int n = steps.size();
    int barWidth = qMin(110, (w - 100) / n);
    int barHeight = qMin(220, h / 3);
    int totalBarsWidth = n * barWidth;
    int startX = (w - totalBarsWidth) / 2;
    int startY = h / 2 - barHeight / 2 - 30;

    for (int i = 0; i < n; ++i) {
        int x = startX + i * barWidth;
        int val = steps[i].first;

        QRect barRect(x + 2, startY, barWidth - 4, barHeight);
        p.fillRect(barRect, QColor(val, val, val));

        p.setPen(QColor(220, 220, 220));
        p.drawRect(barRect);

        p.setPen(QColor(60, 60, 60));
        p.setFont(QFont("SansSerif", 9));
        p.drawText(QRect(x, startY + barHeight + 8, barWidth, 36), Qt::AlignCenter, steps[i].second);
    }

    // Color highlight bars (Red, Green, Blue near white) to check for channel clipping
    int colBarH = 26;
    int colBarY = startY + barHeight + 60;
    int colBarW = totalBarsWidth;

    // Draw RGB gradient stripes near 255
    QLinearGradient rGrad(startX, 0, startX + colBarW, 0);
    rGrad.setColorAt(0, QColor(220, 255, 255));
    rGrad.setColorAt(1, QColor(255, 255, 255));
    p.fillRect(QRect(startX, colBarY, colBarW, colBarH), rGrad);

    QLinearGradient gGrad(startX, 0, startX + colBarW, 0);
    gGrad.setColorAt(0, QColor(255, 220, 255));
    gGrad.setColorAt(1, QColor(255, 255, 255));
    p.fillRect(QRect(startX, colBarY + 30, colBarW, colBarH), gGrad);

    QLinearGradient bGrad(startX, 0, startX + colBarW, 0);
    bGrad.setColorAt(0, QColor(255, 255, 220));
    bGrad.setColorAt(1, QColor(255, 255, 255));
    p.fillRect(QRect(startX, colBarY + 60, colBarW, colBarH), bGrad);
}

void PatternWidget::drawGamma22Pattern(QPainter &p) {
    auto *i18n = I18n::instance();
    int w = width();
    int h = height();

    // 1px alternating horizontal black and white raster background (50% luminance average)
    QImage raster(w, h, QImage::Format_RGB32);
    for (int y = 0; y < h; ++y) {
        QRgb col = (y % 2 == 0) ? qRgb(255, 255, 255) : qRgb(0, 0, 0);
        QRgb *scanLine = reinterpret_cast<QRgb*>(raster.scanLine(y));
        for (int x = 0; x < w; ++x) {
            scanLine[x] = col;
        }
    }
    p.drawImage(0, 0, raster);

    // Dark header bar for text contrast
    p.fillRect(QRect(0, 20, w, 80), QColor(0, 0, 0, 210));

    p.setPen(QColor(255, 255, 255));
    p.setFont(QFont("SansSerif", 14, QFont::Bold));
    p.drawText(QRect(0, 25, w, 30), Qt::AlignCenter, i18n->t("pat_gamma_title"));

    p.setFont(QFont("SansSerif", 10));
    p.setPen(QColor(200, 200, 200));
    p.drawText(QRect(0, 55, w, 35), Qt::AlignCenter, i18n->t("pat_gamma_desc"));

    // Gamma patches: 1.8, 2.0, 2.2 (Standard), 2.4, 2.6
    // Theoretical 50% relative luminance mapped to 8-bit sRGB value: val = 255 * (0.5)^(1/gamma)
    struct GammaRef {
        double gamma;
        int grayVal;
        bool isTarget;
    };

    const QVector<GammaRef> refs = {
        {1.8, qRound(255.0 * qPow(0.5, 1.0 / 1.8)), false},
        {2.0, qRound(255.0 * qPow(0.5, 1.0 / 2.0)), false},
        {2.2, qRound(255.0 * qPow(0.5, 1.0 / 2.2)), true},  // 186
        {2.4, qRound(255.0 * qPow(0.5, 1.0 / 2.4)), false},
        {2.6, qRound(255.0 * qPow(0.5, 1.0 / 2.6)), false}
    };

    int patchSize = qMin(130, (w - 80) / refs.size());
    int totalW = refs.size() * (patchSize + 20) - 20;
    int startX = (w - totalW) / 2;
    int startY = h / 2 - patchSize / 2;

    for (int i = 0; i < refs.size(); ++i) {
        int x = startX + i * (patchSize + 20);
        QRect patchRect(x, startY, patchSize, patchSize);

        // Fill with solid gray
        p.fillRect(patchRect, QColor(refs[i].grayVal, refs[i].grayVal, refs[i].grayVal));

        // Highlight center target patch
        if (refs[i].isTarget) {
            p.setPen(QPen(QColor(59, 130, 246), 3));
            p.drawRect(patchRect);
        }

        // Label box below
        QRect lblRect(x - 10, startY + patchSize + 10, patchSize + 20, 42);
        p.fillRect(lblRect, QColor(0, 0, 0, 200));
        p.setPen(refs[i].isTarget ? QColor(96, 165, 250) : QColor(255, 255, 255));
        p.setFont(QFont("SansSerif", 10, refs[i].isTarget ? QFont::Bold : QFont::Normal));
        QString patchText = i18n->t("pat_gamma_patch_fmt").arg(QString::number(refs[i].gamma, 'f', 1));
        if (refs[i].isTarget) {
            patchText += "\n" + i18n->t("pat_gamma_target");
        }
        p.drawText(lblRect, Qt::AlignCenter, patchText);
    }
}

void PatternWidget::drawGrayRampPattern(QPainter &p) {
    auto *i18n = I18n::instance();
    p.fillRect(rect(), QColor(20, 24, 33));

    int w = width();
    int h = height();

    p.setPen(QColor(240, 240, 240));
    p.setFont(QFont("SansSerif", 13, QFont::Bold));
    p.drawText(QRect(0, 25, w, 28), Qt::AlignCenter, i18n->t("pat_gray_title"));

    p.setFont(QFont("SansSerif", 10));
    p.setPen(QColor(160, 160, 160));
    p.drawText(QRect(0, 55, w, 22), Qt::AlignCenter, i18n->t("pat_gray_desc"));

    int margin = 50;
    int usableW = w - 2 * margin;
    int startY = 95;

    // 1. 32-step grayscale bar
    p.setPen(QColor(180, 180, 180));
    p.setFont(QFont("SansSerif", 9, QFont::Bold));
    p.drawText(QRect(margin, startY, usableW, 20), Qt::AlignLeft, i18n->t("pat_gray_32step"));
    startY += 24;

    int steps = 32;
    int stepW = usableW / steps;
    int barH = 45;
    for (int i = 0; i < steps; ++i) {
        int val = qRound((i / 31.0) * 255.0);
        p.fillRect(QRect(margin + i * stepW, startY, stepW, barH), QColor(val, val, val));
    }
    p.setPen(QColor(60, 60, 60));
    p.drawRect(margin, startY, steps * stepW, barH);
    startY += barH + 20;

    // 2. Smooth continuous grayscale gradient
    p.setPen(QColor(180, 180, 180));
    p.drawText(QRect(margin, startY, usableW, 20), Qt::AlignLeft, i18n->t("pat_gray_smooth"));
    startY += 24;

    QLinearGradient grayGrad(margin, 0, margin + usableW, 0);
    grayGrad.setColorAt(0, QColor(0, 0, 0));
    grayGrad.setColorAt(1, QColor(255, 255, 255));
    p.fillRect(QRect(margin, startY, usableW, barH), grayGrad);
    p.setPen(QColor(60, 60, 60));
    p.drawRect(margin, startY, usableW, barH);
    startY += barH + 20;

    // 3. RGB Channel Linearity
    p.setPen(QColor(180, 180, 180));
    p.drawText(QRect(margin, startY, usableW, 20), Qt::AlignLeft, i18n->t("pat_gray_rgb"));
    startY += 24;

    int rgbH = 22;
    // Red
    QLinearGradient rGrad(margin, 0, margin + usableW, 0);
    rGrad.setColorAt(0, QColor(0, 0, 0));
    rGrad.setColorAt(1, QColor(255, 0, 0));
    p.fillRect(QRect(margin, startY, usableW, rgbH), rGrad);

    // Green
    QLinearGradient gGrad(margin, 0, margin + usableW, 0);
    gGrad.setColorAt(0, QColor(0, 0, 0));
    gGrad.setColorAt(1, QColor(0, 255, 0));
    p.fillRect(QRect(margin, startY + rgbH + 6, usableW, rgbH), gGrad);

    // Blue
    QLinearGradient bGrad(margin, 0, margin + usableW, 0);
    bGrad.setColorAt(0, QColor(0, 0, 0));
    bGrad.setColorAt(1, QColor(0, 0, 255));
    p.fillRect(QRect(margin, startY + 2 * (rgbH + 6), usableW, rgbH), bGrad);
}

void PatternWidget::drawColorUniformityPattern(QPainter &p) {
    auto *i18n = I18n::instance();
    QColor fillCol;
    QString colorName;

    switch (m_uniformityColor) {
    case UniformityColor::White:
        fillCol = QColor(255, 255, 255);
        colorName = i18n->t("color_white");
        break;
    case UniformityColor::Gray50:
        fillCol = QColor(128, 128, 128);
        colorName = i18n->t("color_gray50");
        break;
    case UniformityColor::Black:
        fillCol = QColor(0, 0, 0);
        colorName = i18n->t("color_black");
        break;
    case UniformityColor::Red:
        fillCol = QColor(255, 0, 0);
        colorName = i18n->t("color_red");
        break;
    case UniformityColor::Green:
        fillCol = QColor(0, 255, 0);
        colorName = i18n->t("color_green");
        break;
    case UniformityColor::Blue:
        fillCol = QColor(0, 0, 255);
        colorName = i18n->t("color_blue");
        break;
    case UniformityColor::Cyan:
        fillCol = QColor(0, 255, 255);
        colorName = i18n->t("color_cyan");
        break;
    case UniformityColor::Magenta:
        fillCol = QColor(255, 0, 255);
        colorName = i18n->t("color_magenta");
        break;
    case UniformityColor::Yellow:
        fillCol = QColor(255, 255, 0);
        colorName = i18n->t("color_yellow");
        break;
    }

    p.fillRect(rect(), fillCol);

    // Subtle small tag in top left indicating current color & hint
    QRect tagRect(20, 20, 360, 40);
    p.fillRect(tagRect, QColor(0, 0, 0, 180));
    p.setPen(QColor(255, 255, 255));
    p.setFont(QFont("SansSerif", 10, QFont::Bold));
    p.drawText(QRect(30, 24, 340, 18), Qt::AlignLeft, QString("%1: %2").arg(i18n->t("step6_short"), colorName));
    p.setFont(QFont("SansSerif", 8));
    p.setPen(QColor(200, 200, 200));
    p.drawText(QRect(30, 42, 340, 16), Qt::AlignLeft, i18n->t("color_unif_hint"));
}

void PatternWidget::drawSharpnessPattern(QPainter &p) {
    auto *i18n = I18n::instance();
    p.fillRect(rect(), QColor(128, 128, 128));

    int w = width();
    int h = height();

    p.setPen(QColor(255, 255, 255));
    p.setFont(QFont("SansSerif", 14, QFont::Bold));
    p.drawText(QRect(0, 35, w, 30), Qt::AlignCenter, i18n->t("pat_sharp_title"));

    p.setFont(QFont("SansSerif", 10));
    p.setPen(QColor(230, 230, 230));
    p.drawText(QRect(0, 65, w, 25), Qt::AlignCenter, i18n->t("pat_sharp_desc"));

    int boxSize = 160;
    int centerX = w / 2;
    int centerY = h / 2 + 10;

    // 1px alternating horizontal lines box
    QImage hLines(boxSize, boxSize, QImage::Format_RGB32);
    for (int y = 0; y < boxSize; ++y) {
        QRgb col = (y % 2 == 0) ? qRgb(255, 255, 255) : qRgb(0, 0, 0);
        QRgb *line = reinterpret_cast<QRgb*>(hLines.scanLine(y));
        for (int x = 0; x < boxSize; ++x) line[x] = col;
    }
    p.drawImage(centerX - boxSize - 20, centerY - boxSize / 2, hLines);

    // 1px alternating vertical lines box
    QImage vLines(boxSize, boxSize, QImage::Format_RGB32);
    for (int y = 0; y < boxSize; ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(vLines.scanLine(y));
        for (int x = 0; x < boxSize; ++x) {
            line[x] = (x % 2 == 0) ? qRgb(255, 255, 255) : qRgb(0, 0, 0);
        }
    }
    p.drawImage(centerX + 20, centerY - boxSize / 2, vLines);

    // Checkerboard 1px box
    int chkSize = 120;
    QImage chkImg(chkSize, chkSize, QImage::Format_RGB32);
    for (int y = 0; y < chkSize; ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(chkImg.scanLine(y));
        for (int x = 0; x < chkSize; ++x) {
            line[x] = ((x + y) % 2 == 0) ? qRgb(255, 255, 255) : qRgb(0, 0, 0);
        }
    }
    p.drawImage(centerX - chkSize / 2, centerY + boxSize / 2 + 30, chkImg);

    // Text sharpness sample
    p.setPen(QColor(0, 0, 0));
    p.setFont(QFont("Monospace", 12, QFont::Bold));
    p.drawText(QRect(0, centerY - boxSize / 2 - 50, w, 30), Qt::AlignCenter, "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG 0123456789");
}

void PatternWidget::drawGeometryPattern(QPainter &p) {
    auto *i18n = I18n::instance();
    p.fillRect(rect(), QColor(0, 0, 0));

    int w = width();
    int h = height();

    // 1px outer frame
    p.setPen(QColor(255, 255, 255));
    p.drawRect(0, 0, w - 1, h - 1);

    // Grid lines (every 50px)
    p.setPen(QColor(45, 45, 45));
    for (int x = 0; x < w; x += 50) p.drawLine(x, 0, x, h);
    for (int y = 0; y < h; y += 50) p.drawLine(0, y, w, y);

    // Center cross
    p.setPen(QColor(59, 130, 246));
    p.drawLine(w / 2, 0, w / 2, h);
    p.drawLine(0, h / 2, w, h / 2);

    // Center and corner circles
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(QPen(QColor(239, 68, 68), 2));
    int radius = qMin(w, h) / 4;
    p.drawEllipse(QPoint(w / 2, h / 2), radius, radius);

    // Corner circles
    int cr = 60;
    p.setPen(QPen(QColor(16, 185, 129), 2));
    p.drawEllipse(QPoint(cr, cr), cr, cr);
    p.drawEllipse(QPoint(w - cr, cr), cr, cr);
    p.drawEllipse(QPoint(cr, h - cr), cr, cr);
    p.drawEllipse(QPoint(w - cr, h - cr), cr, cr);

    p.setPen(QColor(255, 255, 255));
    p.setFont(QFont("SansSerif", 12, QFont::Bold));
    p.drawText(QRect(0, h / 2 - 30, w, 25), Qt::AlignCenter, i18n->t("pat_geom_title"));
}

void PatternWidget::drawHudOverlay(QPainter &p) {
    auto *i18n = I18n::instance();
    int w = width();
    int h = height();

    p.setRenderHint(QPainter::Antialiasing, true);

    // Bottom Guide Floating Card
    int cardW = qMin(760, w - 40);
    int cardH = 150;
    int cardX = (w - cardW) / 2;
    int cardY = h - cardH - 30;

    QPainterPath cardPath;
    cardPath.addRoundedRect(cardX, cardY, cardW, cardH, 16, 16);

    p.fillPath(cardPath, QColor(15, 23, 42, 235)); // Slate 900
    p.setPen(QPen(QColor(59, 130, 246, 180), 1.5));
    p.drawPath(cardPath);

    // Header in Card (Step title + badge)
    int contentX = cardX + 24;
    int contentY = cardY + 20;

    QString stepBadge = (m_currentStep >= 0 && m_totalSteps > 0)
                            ? i18n->t("hud_step_prefix").arg(m_currentStep + 1).arg(m_totalSteps)
                            : "";
    p.setPen(QColor(96, 165, 250)); // Blue 400
    p.setFont(QFont("SansSerif", 13, QFont::Bold));
    p.drawText(QRect(contentX, contentY, cardW - 180, 26), Qt::AlignLeft | Qt::AlignVCenter, stepBadge + m_guideTitle);

    // Instruction Text
    p.setPen(QColor(226, 232, 240)); // Slate 200
    p.setFont(QFont("SansSerif", 10));
    p.drawText(QRect(contentX, contentY + 28, cardW - 180, 44), Qt::AlignLeft | Qt::TextWordWrap, m_guideInstruction);

    // OSD Tip
    if (!m_guideOsdTip.isEmpty()) {
        p.setPen(QColor(251, 191, 36)); // Amber 400
        p.setFont(QFont("SansSerif", 9, QFont::Bold));
        p.drawText(QRect(contentX, contentY + 74, cardW - 180, 24), Qt::AlignLeft | Qt::AlignVCenter, i18n->t("hud_osd_tip") + m_guideOsdTip);
    }

    // Shortcut hints at card bottom
    p.setPen(QColor(148, 163, 184)); // Slate 400
    p.setFont(QFont("SansSerif", 8));
    QString shortcuts = i18n->t("hud_shortcuts");
    if (m_patternType == PatternType::ColorUniformity) {
        shortcuts = i18n->t("hud_shortcuts_color") + shortcuts;
    }
    p.drawText(QRect(contentX, cardY + cardH - 24, cardW - 48, 18), Qt::AlignLeft, shortcuts);

    // Buttons on Card Right Side
    int btnW = 75;
    int btnH = 34;
    int btnY = cardY + (cardH - btnH) / 2 - 10;
    int btnPrevX = cardX + cardW - 2 * btnW - 30;
    int btnNextX = cardX + cardW - btnW - 20;

    m_btnPrevRect = QRect(btnPrevX, btnY, btnW, btnH);
    m_btnNextRect = QRect(btnNextX, btnY, btnW, btnH);

    // Prev Button
    QPainterPath prevBtnPath;
    prevBtnPath.addRoundedRect(m_btnPrevRect, 8, 8);
    p.fillPath(prevBtnPath, QColor(51, 65, 85)); // Slate 700
    p.setPen(QColor(203, 213, 225));
    p.setFont(QFont("SansSerif", 9, QFont::Bold));
    p.drawText(m_btnPrevRect, Qt::AlignCenter, i18n->t("hud_btn_prev"));

    // Next Button
    QPainterPath nextBtnPath;
    nextBtnPath.addRoundedRect(m_btnNextRect, 8, 8);
    p.fillPath(nextBtnPath, QColor(37, 99, 235)); // Blue 600
    p.setPen(QColor(255, 255, 255));
    p.drawText(m_btnNextRect, Qt::AlignCenter, (m_currentStep >= 0 && m_currentStep == m_totalSteps - 1) ? i18n->t("btn_wizard_finish") : i18n->t("hud_btn_next"));

    // Close button (Top-Right of screen)
    m_btnCloseRect = QRect(w - 50, 15, 35, 35);
    QPainterPath closePath;
    closePath.addRoundedRect(m_btnCloseRect, 8, 8);
    p.fillPath(closePath, QColor(15, 23, 42, 200));
    p.setPen(QColor(239, 68, 68));
    p.setFont(QFont("SansSerif", 12, QFont::Bold));
    p.drawText(m_btnCloseRect, Qt::AlignCenter, "✕");
}

void PatternWidget::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Space:
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Right:
        emit nextStepRequested();
        break;
    case Qt::Key_Backspace:
    case Qt::Key_Left:
        emit prevStepRequested();
        break;
    case Qt::Key_Escape:
        emit closeRequested();
        break;
    case Qt::Key_F:
    case Qt::Key_F11:
        emit toggleFullscreenRequested();
        break;
    case Qt::Key_H:
        setHudVisible(!m_hudVisible);
        break;
    case Qt::Key_C:
        if (m_patternType == PatternType::ColorUniformity) {
            cycleUniformityColor();
        }
        break;
    default:
        QWidget::keyPressEvent(event);
        break;
    }
}

void PatternWidget::mouseMoveEvent(QMouseEvent *) {
    if (!m_hudVisible) {
        setHudVisible(true);
    }
    m_hudFadeTimer.start();
}

void PatternWidget::mousePressEvent(QMouseEvent *event) {
    if (m_hudVisible) {
        if (m_btnNextRect.contains(event->pos())) {
            emit nextStepRequested();
            return;
        }
        if (m_btnPrevRect.contains(event->pos())) {
            emit prevStepRequested();
            return;
        }
        if (m_btnCloseRect.contains(event->pos())) {
            emit closeRequested();
            return;
        }
    }

    if (m_patternType == PatternType::ColorUniformity) {
        cycleUniformityColor();
    }
}
