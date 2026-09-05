#pragma once

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include "pattern_types.h"

class PatternWidget : public QWidget {
    Q_OBJECT
public:
    explicit PatternWidget(QWidget *parent = nullptr);
    ~PatternWidget() override;

    void setPatternType(PatternType type);
    PatternType patternType() const { return m_patternType; }

    void setUniformityColor(UniformityColor color);
    UniformityColor uniformityColor() const { return m_uniformityColor; }
    void cycleUniformityColor();

    void setGuideInfo(const QString &title, const QString &instruction, const QString &osdTip);
    void setHudVisible(bool visible);
    void setStepNavigation(int currentStep, int totalSteps);

signals:
    void nextStepRequested();
    void prevStepRequested();
    void closeRequested();
    void toggleFullscreenRequested();
    void uniformityColorChanged(UniformityColor color);

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    void drawBlackLevelPattern(QPainter &p);
    void drawWhiteLevelPattern(QPainter &p);
    void drawGamma22Pattern(QPainter &p);
    void drawGrayRampPattern(QPainter &p);
    void drawColorUniformityPattern(QPainter &p);
    void drawSharpnessPattern(QPainter &p);
    void drawGeometryPattern(QPainter &p);
    void drawHudOverlay(QPainter &p);

    PatternType m_patternType = PatternType::BlackLevel;
    UniformityColor m_uniformityColor = UniformityColor::White;

    bool m_hudVisible = true;
    QTimer m_hudFadeTimer;
    int m_currentStep = -1;
    int m_totalSteps = -1;

    QString m_guideTitle;
    QString m_guideInstruction;
    QString m_guideOsdTip;

    int m_blinkPhase = 0;
    QTimer m_blinkTimer;

    QRect m_btnNextRect;
    QRect m_btnPrevRect;
    QRect m_btnCloseRect;
    QRect m_btnColorRect;
};
