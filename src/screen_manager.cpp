#include "screen_manager.h"
#include <QGuiApplication>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPainterPath>

namespace {
class IdentifyBannerWidget : public QWidget {
public:
    IdentifyBannerWidget(const ScreenInfo &info, QWidget *parent = nullptr)
        : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::SubWindow | Qt::Tool) {
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_ShowWithoutActivating);
        setAttribute(Qt::WA_DeleteOnClose);

        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(40, 30, 40, 30);
        layout->setAlignment(Qt::AlignCenter);

        auto *numLabel = new QLabel(QString("🖥️ ディスプレイ %1").arg(info.index + 1), this);
        numLabel->setStyleSheet("color: #60a5fa; font-size: 32px; font-weight: bold;");
        numLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(numLabel);

        QString detail = QString("%1 (%2x%3 @ %4Hz)\n%5")
                             .arg(info.name)
                             .arg(info.geometry.width())
                             .arg(info.geometry.height())
                             .arg(info.refreshRate)
                             .arg(info.isPrimary ? "[プライマリディスプレイ]" : "");
        auto *detailLabel = new QLabel(detail.trimmed(), this);
        detailLabel->setStyleSheet("color: #e2e8f0; font-size: 18px; font-weight: 500;");
        detailLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(detailLabel);

        resize(460, 180);
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        QPainterPath path;
        path.addRoundedRect(rect().adjusted(2, 2, -2, -2), 16, 16);

        QColor bg(15, 23, 42, 235); // Slate 900 translucent
        painter.fillPath(path, bg);

        QPen pen(QColor(59, 130, 246, 200), 2); // Blue 500 border
        painter.setPen(pen);
        painter.drawPath(path);
    }
};
}

ScreenManager::ScreenManager(QObject *parent) : QObject(parent) {
    connect(qApp, &QGuiApplication::screenAdded, this, &ScreenManager::onScreenAdded);
    connect(qApp, &QGuiApplication::screenRemoved, this, &ScreenManager::onScreenRemoved);
}

ScreenManager::~ScreenManager() = default;

QVector<ScreenInfo> ScreenManager::getScreenList() const {
    QVector<ScreenInfo> list;
    const auto screens = QGuiApplication::screens();
    const auto *primary = QGuiApplication::primaryScreen();

    for (int i = 0; i < screens.size(); ++i) {
        auto *s = screens[i];
        if (!s) continue;

        ScreenInfo info;
        info.index = i;
        info.name = s->name();
        info.model = s->model().isEmpty() ? s->name() : s->model();
        info.manufacturer = s->manufacturer();
        info.geometry = s->geometry();
        info.refreshRate = qRound(s->refreshRate());
        info.devicePixelRatio = s->devicePixelRatio();
        info.physicalDotsPerInch = s->physicalDotsPerInch();
        info.logicalDotsPerInch = s->logicalDotsPerInch();
        info.depth = s->depth();
        info.isPrimary = (s == primary);

        list.append(info);
    }
    return list;
}

QScreen* ScreenManager::getScreen(int index) const {
    const auto screens = QGuiApplication::screens();
    if (index >= 0 && index < screens.size()) {
        return screens[index];
    }
    return QGuiApplication::primaryScreen();
}

int ScreenManager::getPrimaryScreenIndex() const {
    const auto screens = QGuiApplication::screens();
    const auto *primary = QGuiApplication::primaryScreen();
    for (int i = 0; i < screens.size(); ++i) {
        if (screens[i] == primary) {
            return i;
        }
    }
    return 0;
}

void ScreenManager::showIdentifyOverlay(int screenIndex, int durationMs) {
    auto screens = getScreenList();
    if (screenIndex < 0 || screenIndex >= screens.size()) return;

    auto *banner = new IdentifyBannerWidget(screens[screenIndex]);
    const auto &geom = screens[screenIndex].geometry;
    int x = geom.x() + (geom.width() - banner->width()) / 2;
    int y = geom.y() + (geom.height() - banner->height()) / 2;
    banner->move(x, y);
    banner->show();

    QTimer::singleShot(durationMs, banner, &QWidget::close);
}

void ScreenManager::showIdentifyOverlayAll(int durationMs) {
    auto screens = getScreenList();
    for (int i = 0; i < screens.size(); ++i) {
        showIdentifyOverlay(i, durationMs);
    }
}

void ScreenManager::onScreenAdded(QScreen *) {
    emit screensChanged();
}

void ScreenManager::onScreenRemoved(QScreen *) {
    emit screensChanged();
}
