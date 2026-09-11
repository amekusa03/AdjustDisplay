#include "screen_manager.h"
#include "i18n.h"
#include <QGuiApplication>
#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>
#include <QDebug>

ScreenManager::ScreenManager(QObject *parent) : QObject(parent) {
    connect(qGuiApp, &QGuiApplication::screenAdded, this, &ScreenManager::onScreenAdded);
    connect(qGuiApp, &QGuiApplication::screenRemoved, this, &ScreenManager::onScreenRemoved);
}

ScreenManager::~ScreenManager() = default;

QVector<ScreenInfo> ScreenManager::getScreenList() const {
    QVector<ScreenInfo> list;
    const auto screens = QGuiApplication::screens();
    QScreen *primary = QGuiApplication::primaryScreen();

    for (int i = 0; i < screens.size(); ++i) {
        QScreen *s = screens[i];
        ScreenInfo info;
        info.index = i;
        info.name = s->name();
        info.model = s->model();
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
    return nullptr;
}

int ScreenManager::getPrimaryScreenIndex() const {
    const auto screens = QGuiApplication::screens();
    QScreen *primary = QGuiApplication::primaryScreen();
    for (int i = 0; i < screens.size(); ++i) {
        if (screens[i] == primary) {
            return i;
        }
    }
    return 0;
}

void ScreenManager::showIdentifyOverlay(int screenIndex, int durationMs) {
    QScreen *target = getScreen(screenIndex);
    if (!target) return;

    auto *i18n = I18n::instance();
    auto screens = getScreenList();
    const auto &info = screens[screenIndex];

    // Create an identification banner
    auto *banner = new QWidget(nullptr, Qt::ToolTip | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    banner->setAttribute(Qt::WA_TranslucentBackground, true);
    banner->setAttribute(Qt::WA_DeleteOnClose, true);

    auto *layout = new QVBoxLayout(banner);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *frame = new QFrame(banner);
    frame->setStyleSheet(
        "background-color: rgba(15, 23, 42, 0.92);"
        "border: 3px solid #3b82f6;"
        "border-radius: 20px;"
        "padding: 24px;"
    );
    auto *frameLayout = new QVBoxLayout(frame);
    frameLayout->setAlignment(Qt::AlignCenter);

    auto *numLabel = new QLabel(QString::number(screenIndex + 1), frame);
    numLabel->setStyleSheet("font-size: 80px; font-weight: bold; color: #60a5fa; margin: 0;");
    numLabel->setAlignment(Qt::AlignCenter);
    frameLayout->addWidget(numLabel);

    QString nameText = i18n->t("screen_id_banner")
                           .arg(screenIndex + 1)
                           .arg(info.name)
                           .arg(info.geometry.width())
                           .arg(info.geometry.height())
                           .arg(info.refreshRate);
    auto *nameLabel = new QLabel(nameText, frame);
    nameLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #f8fafc; margin-top: 8px;");
    nameLabel->setAlignment(Qt::AlignCenter);
    frameLayout->addWidget(nameLabel);

    if (info.isPrimary) {
        auto *primLabel = new QLabel(i18n->t("screen_id_primary"), frame);
        primLabel->setStyleSheet("font-size: 14px; color: #34d399; font-weight: 500;");
        primLabel->setAlignment(Qt::AlignCenter);
        frameLayout->addWidget(primLabel);
    }

    layout->addWidget(frame);

    banner->adjustSize();
    QRect geo = target->geometry();
    int x = geo.x() + (geo.width() - banner->width()) / 2;
    int y = geo.y() + (geo.height() - banner->height()) / 2;
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
