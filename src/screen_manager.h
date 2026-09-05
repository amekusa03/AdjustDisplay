#pragma once

#include <QObject>
#include <QScreen>
#include <QVector>
#include <QRect>
#include <QString>

struct ScreenInfo {
    int index;
    QString name;
    QString model;
    QString manufacturer;
    QRect geometry;
    int refreshRate;
    qreal devicePixelRatio;
    qreal physicalDotsPerInch;
    qreal logicalDotsPerInch;
    int depth;
    bool isPrimary;
};

class ScreenManager : public QObject {
    Q_OBJECT
public:
    explicit ScreenManager(QObject *parent = nullptr);
    ~ScreenManager() override;

    QVector<ScreenInfo> getScreenList() const;
    QScreen* getScreen(int index) const;
    int getPrimaryScreenIndex() const;

    void showIdentifyOverlay(int screenIndex, int durationMs = 2500);
    void showIdentifyOverlayAll(int durationMs = 2500);

signals:
    void screensChanged();

private slots:
    void onScreenAdded(QScreen *screen);
    void onScreenRemoved(QScreen *screen);

private:
    void updateScreenList();
};
