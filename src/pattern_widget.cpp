#include "pattern_widget.h"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainterPath>
#include <QFontMetrics>
#include <cmath>

PatternWidget::PatternWidget(QWidget *parent) : QWidget(parent) {
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_OpaquePaintEvent);

    m_hudFadeTimer.setSingleShot(true);
    m_hudFadeTimer.setInterval(5000); // 5 seconds of inactivity fades HUD
    connect(&m_hudFadeTimer, &QTimer::timeout, this, [this]() {
        // When in wizard mode, we can keep HUD subtle or toggleable
        update();
    });

    m_blinkTimer.setInterval(600);
    connect(&m_blinkTimer, &QTimer::timeout, this, [this]() {
        m_blinkPhase = (m_blinkPhase + 1) % 2;
        if (m_patternType == PatternType::BlackLevel || m_patternType == PatternType::WhiteLevel) {
            update();
        }
    });
    m_blinkTimer.start();
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
    p.drawText(QRect(0, 40, w, 30), Qt::AlignCenter, "黒レベル / 輝度 (Brightness) 調整パターン");

    p.setFont(QFont("SansSerif", 11));
    p.setPen(QColor(120, 120, 120));
    p.drawText(QRect(0, 75, w, 25), Qt::AlignCenter, "※ 0%（完全な黒）は背景と同化し、1%〜2%が「かろうじて識別できる」状態にモニタの輝度（Brightness）を調整してください。");

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
    p.drawText(QRect(0, blinkY + boxSize + 10, w, 20), Qt::AlignCenter, "点滅テストボックス (RGB 2 / RGB 4 - 点滅が視認できれば暗部階調は良好)");
}

void PatternWidget::drawWhiteLevelPattern(QPainter &p) {
    // Pure white background (RGB 255, 255, 255)
    p.fillRect(rect(), QColor(255, 255, 255));

    int w = width();
    int h = height();

    p.setPen(QColor(50, 50, 50));
    p.setFont(QFont("SansSerif", 14, QFont::Bold));
    p.drawText(QRect(0, 40, w, 30), Qt::AlignCenter, "白レベル / コントラスト (Contrast) 調整パターン");

    p.setFont(QFont("SansSerif", 11));
    p.setPen(QColor(90, 90, 90));
    p.drawText(QRect(0, 75, w, 25), Qt::AlignCenter, "※ 100%（完全な白）と99%〜99.6%（RGB 254）の境界が識別できる限界までモニタのコントラストを調整してください。");

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

    p.setPen(QColor(100, 100, 100));
    p.drawText(QRect(0, colBarY + 95, w, 20), Qt::AlignCenter, "各色ハイライト階調 (RGB各色で白飛び・色転びが生じていないか確認)");
}

void PatternWidget::drawGamma22Pattern(QPainter &p) {
    p.fillRect(rect(), QColor(18, 18, 18));

    int w = width();
    int h = height();

    p.setPen(QColor(240, 240, 240));
    p.setFont(QFont("SansSerif", 14, QFont::Bold));
    p.drawText(QRect(0, 30, w, 30), Qt::AlignCenter, "ガンマ 2.2 (Gamma 2.2) 調整パターン");

    p.setFont(QFont("SansSerif", 11));
    p.setPen(QColor(170, 170, 170));
    p.drawText(QRect(0, 65, w, 25), Qt::AlignCenter, "※ 画面から1mほど離れるか目を細めて見たとき、中央の「2.2」パッチが白黒ストライプ背景と同化して見えるのが理想です。");

    int patternW = qMin(720, w - 80);
    int patternH = qMin(320, h - 220);
    int startX = (w - patternW) / 2;
    int startY = (h - patternH) / 2;

    // Background 1-pixel alternating black/white raster
    // Create alternating line pattern image for high performance & accuracy
    QImage rasterImg(patternW, patternH, QImage::Format_RGB32);
    for (int y = 0; y < patternH; ++y) {
        QRgb color = (y % 2 == 0) ? qRgb(255, 255, 255) : qRgb(0, 0, 0);
        QRgb *scanLine = reinterpret_cast<QRgb*>(rasterImg.scanLine(y));
        for (int x = 0; x < patternW; ++x) {
            scanLine[x] = color;
        }
    }
    p.drawImage(startX, startY, rasterImg);

    // Overlaid Gamma Comparison Patches
    // Linear 50% luminance: RGB = 255 * (0.5)^(1/gamma)
    const QVector<QPair<double, QString>> gammaSteps = {
        {1.8, "γ 1.8\n(174)"},
        {2.0, "γ 2.0\n(180)"},
        {2.2, "γ 2.2 ★\n(186)"},
        {2.4, "γ 2.4\n(191)"},
        {2.6, "γ 2.6\n(196)"}
    };

    int patchCount = gammaSteps.size();
    int patchW = patternW / (patchCount * 2);
    int patchH = patternH * 3 / 4;
    int patchY = startY + (patternH - patchH) / 2;

    for (int i = 0; i < patchCount; ++i) {
        double g = gammaSteps[i].first;
        int rgb = qRound(255.0 * std::pow(0.5, 1.0 / g));
        int px = startX + (2 * i + 1) * (patternW / (patchCount * 2 + 1));

        QRect patchRect(px, patchY, patchW, patchH);
        p.fillRect(patchRect, QColor(rgb, rgb, rgb));
        p.setPen(g == 2.2 ? QColor(59, 130, 246) : QColor(80, 80, 80));
        p.drawRect(patchRect);

        // Label above or inside patch
        p.setPen(QColor(0, 0, 0));
        p.setFont(QFont("SansSerif", 10, g == 2.2 ? QFont::Bold : QFont::Normal));
        p.drawText(patchRect, Qt::AlignCenter, gammaSteps[i].second);
    }

    p.setPen(QColor(140, 140, 140));
    p.setFont(QFont("SansSerif", 10));
    p.drawText(QRect(0, startY + patternH + 15, w, 25), Qt::AlignCenter, "※ ガンマがズレている場合は、モニタOSDの「ガンマ (Gamma)」設定または「カラーモード」を変更してください。");
}

void PatternWidget::drawGrayRampPattern(QPainter &p) {
    p.fillRect(rect(), QColor(24, 24, 27));

    int w = width();
    int h = height();

    p.setPen(QColor(240, 240, 240));
    p.setFont(QFont("SansSerif", 14, QFont::Bold));
    p.drawText(QRect(0, 30, w, 30), Qt::AlignCenter, "グレースケール & カラーバランス (Grayscale & Color Ramp)");

    p.setFont(QFont("SansSerif", 11));
    p.setPen(QColor(160, 160, 160));
    p.drawText(QRect(0, 65, w, 25), Qt::AlignCenter, "※ 各階調が滑らかに変化し、中間に緑やピンク等の不自然な色被りがないことを確認してください。");

    int rampW = qMin(800, w - 80);
    int startX = (w - rampW) / 2;
    int currentY = 110;

    // 1. 32-step grayscale bar
    int stepBarH = 45;
    int steps = 32;
    int stepW = rampW / steps;
    for (int i = 0; i < steps; ++i) {
        int val = (i * 255) / (steps - 1);
        p.fillRect(QRect(startX + i * stepW, currentY, stepW, stepBarH), QColor(val, val, val));
    }
    p.setPen(QColor(80, 80, 80));
    p.drawRect(startX, currentY, steps * stepW, stepBarH);
    p.setPen(QColor(180, 180, 180));
    p.setFont(QFont("SansSerif", 9));
    p.drawText(QRect(startX, currentY + stepBarH + 2, rampW, 20), Qt::AlignLeft, "32階調 ステップバー");

    currentY += stepBarH + 30;

    // 2. Smooth continuous grayscale gradient
    int smoothBarH = 45;
    QLinearGradient grayGrad(startX, 0, startX + rampW, 0);
    grayGrad.setColorAt(0, QColor(0, 0, 0));
    grayGrad.setColorAt(1, QColor(255, 255, 255));
    p.fillRect(QRect(startX, currentY, rampW, smoothBarH), grayGrad);
    p.setPen(QColor(80, 80, 80));
    p.drawRect(startX, currentY, rampW, smoothBarH);
    p.setPen(QColor(180, 180, 180));
    p.drawText(QRect(startX, currentY + smoothBarH + 2, rampW, 20), Qt::AlignLeft, "連続無段階 グレースケール (バンディング・階調跳びの確認)");

    currentY += smoothBarH + 30;

    // 3. Red, Green, Blue individual gradients
    int rgbBarH = 26;
    // Red
    QLinearGradient rGrad(startX, 0, startX + rampW, 0);
    rGrad.setColorAt(0, QColor(0, 0, 0));
    rGrad.setColorAt(1, QColor(255, 0, 0));
    p.fillRect(QRect(startX, currentY, rampW, rgbBarH), rGrad);
    currentY += rgbBarH + 4;

    // Green
    QLinearGradient gGrad(startX, 0, startX + rampW, 0);
    gGrad.setColorAt(0, QColor(0, 0, 0));
    gGrad.setColorAt(1, QColor(0, 255, 0));
    p.fillRect(QRect(startX, currentY, rampW, rgbBarH), gGrad);
    currentY += rgbBarH + 4;

    // Blue
    QLinearGradient bGrad(startX, 0, startX + rampW, 0);
    bGrad.setColorAt(0, QColor(0, 0, 0));
    bGrad.setColorAt(1, QColor(0, 0, 255));
    p.fillRect(QRect(startX, currentY, rampW, rgbBarH), bGrad);
    currentY += rgbBarH + 4;

    p.setPen(QColor(180, 180, 180));
    p.drawText(QRect(startX, currentY + 2, rampW, 20), Qt::AlignLeft, "RGB個別チャンネル階調 (各原色のリニアリティ確認)");
}

void PatternWidget::drawColorUniformityPattern(QPainter &p) {
    QColor fill;
    QString colorName;
    switch (m_uniformityColor) {
    case UniformityColor::White: fill = QColor(255, 255, 255); colorName = "ホワイト (100% White)"; break;
    case UniformityColor::Gray50: fill = QColor(128, 128, 128); colorName = "ニュートラルグレー (50% Gray)"; break;
    case UniformityColor::Black: fill = QColor(0, 0, 0); colorName = "ブラック (0% Black)"; break;
    case UniformityColor::Red: fill = QColor(255, 0, 0); colorName = "レッド (Pure Red)"; break;
    case UniformityColor::Green: fill = QColor(0, 255, 0); colorName = "グリーン (Pure Green)"; break;
    case UniformityColor::Blue: fill = QColor(0, 0, 255); colorName = "ブルー (Pure Blue)"; break;
    case UniformityColor::Cyan: fill = QColor(0, 255, 255); colorName = "シアン (Pure Cyan)"; break;
    case UniformityColor::Magenta: fill = QColor(255, 0, 255); colorName = "マゼンタ (Pure Magenta)"; break;
    case UniformityColor::Yellow: fill = QColor(255, 255, 0); colorName = "イエロー (Pure Yellow)"; break;
    }

    p.fillRect(rect(), fill);

    // Subtle center hint if HUD is hidden
    if (!m_hudVisible) {
        QColor textCol = (m_uniformityColor == UniformityColor::White || m_uniformityColor == UniformityColor::Yellow || m_uniformityColor == UniformityColor::Cyan)
                             ? QColor(100, 100, 100, 160)
                             : QColor(200, 200, 200, 160);
        p.setPen(textCol);
        p.setFont(QFont("SansSerif", 11));
        p.drawText(QRect(0, height() - 40, width(), 30), Qt::AlignCenter, QString("色均一性 / ドット抜け検査: %1 ( [C] キーで色切り替え )").arg(colorName));
    }
}

void PatternWidget::drawSharpnessPattern(QPainter &p) {
    p.fillRect(rect(), QColor(128, 128, 128));

    int w = width();
    int h = height();

    p.setPen(QColor(255, 255, 255));
    p.setFont(QFont("SansSerif", 14, QFont::Bold));
    p.drawText(QRect(0, 30, w, 30), Qt::AlignCenter, "シャープネス / フォーカス (Sharpness & Moire) 調整パターン");

    p.setFont(QFont("SansSerif", 11));
    p.setPen(QColor(230, 230, 230));
    p.drawText(QRect(0, 65, w, 25), Qt::AlignCenter, "※ 文字や線の輪郭に不自然な白フチ（オーバーシュート/リンギング）やモアレがないか確認してください。");

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
    p.drawText(QRect(0, h / 2 - 30, w, 25), Qt::AlignCenter, "画面比率 & オーバースキャン (1:1 ピクセルマッピング) 検査");
}

void PatternWidget::drawHudOverlay(QPainter &p) {
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
                            ? QString("ステップ %1 / %2 : ").arg(m_currentStep + 1).arg(m_totalSteps)
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
        p.drawText(QRect(contentX, contentY + 74, cardW - 180, 24), Qt::AlignLeft | Qt::AlignVCenter, "💡 OSD操作: " + m_guideOsdTip);
    }

    // Shortcut hints at card bottom
    p.setPen(QColor(148, 163, 184)); // Slate 400
    p.setFont(QFont("SansSerif", 8));
    QString shortcuts = "[Space/Enter] 次へ   [Backspace] 前へ   [H] ガイド表示切替   [F] 全画面   [Esc] 戻る";
    if (m_patternType == PatternType::ColorUniformity) {
        shortcuts = "[C] 色切り替え   " + shortcuts;
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
    p.drawText(m_btnPrevRect, Qt::AlignCenter, "◀ 前へ");

    // Next Button
    QPainterPath nextBtnPath;
    nextBtnPath.addRoundedRect(m_btnNextRect, 8, 8);
    p.fillPath(nextBtnPath, QColor(37, 99, 235)); // Blue 600
    p.setPen(QColor(255, 255, 255));
    p.drawText(m_btnNextRect, Qt::AlignCenter, "次へ ▶");

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
