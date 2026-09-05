#include "main_window.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QScrollArea>
#include <QFrame>
#include <QMessageBox>
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QIcon>
#include <QStyle>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_screenManager(new ScreenManager(this)),
      m_hardwareBridge(new HardwareBridge(this)),
      m_wizard(new WizardController(this)) {
    
    setWindowTitle("Ubuntu ディスプレイ調整ツール (AdjustDisplay)");
    resize(1020, 700);
    setMinimumSize(880, 600);

    setupUi();
    applyTheme();

    connect(m_screenManager, &ScreenManager::screensChanged, this, &MainWindow::refreshScreenList);
    connect(m_wizard, &WizardController::stepChanged, this, &MainWindow::onWizardStepChanged);
    connect(m_wizard, &WizardController::wizardFinished, this, [this]() {
        QMessageBox::information(this, "調整完了", "すべての調整ステップが完了しました！\n快適なディスプレイ表示でお楽しみください。");
    });

    connect(m_hardwareBridge, &HardwareBridge::capabilitiesUpdated, this, &MainWindow::onHardwareCapsUpdated);

    refreshScreenList();
    m_wizard->reset();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi() {
    auto *central = new QWidget(this);
    setCentralWidget(central);

    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(18, 16, 18, 16);
    mainLayout->setSpacing(14);

    // 1. Top Header Bar (Monitor Selection & Actions)
    auto *headerFrame = new QFrame(this);
    headerFrame->setObjectName("headerFrame");
    auto *headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setContentsMargins(14, 10, 14, 10);
    headerLayout->setSpacing(12);

    auto *logoLabel = new QLabel("🖥️", this);
    logoLabel->setStyleSheet("font-size: 26px;");
    headerLayout->addWidget(logoLabel);

    auto *titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(2);
    auto *appTitle = new QLabel("Ubuntu ディスプレイ調整ツール", this);
    appTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #f8fafc;");
    auto *appSub = new QLabel("高精度キャリブレーション & テストパターン", this);
    appSub->setStyleSheet("font-size: 11px; color: #94a3b8;");
    titleLayout->addWidget(appTitle);
    titleLayout->addWidget(appSub);
    headerLayout->addLayout(titleLayout);

    headerLayout->addStretch(1);

    auto *screenSelectLabel = new QLabel("対象ディスプレイ:", this);
    screenSelectLabel->setStyleSheet("font-weight: bold; color: #cbd5e1;");
    headerLayout->addWidget(screenSelectLabel);

    m_screenCombo = new QComboBox(this);
    m_screenCombo->setMinimumWidth(260);
    headerLayout->addWidget(m_screenCombo);

    m_btnIdentify = new QPushButton("🎯 画面識別", this);
    m_btnIdentify->setToolTip("選択中の画面に大きな識別番号を表示します");
    headerLayout->addWidget(m_btnIdentify);

    m_btnIdentifyAll = new QPushButton("✨ 全画面識別", this);
    m_btnIdentifyAll->setToolTip("すべての画面に識別番号を同時に表示します");
    headerLayout->addWidget(m_btnIdentifyAll);

    m_btnRefreshScreens = new QPushButton("🔄 再検出", this);
    headerLayout->addWidget(m_btnRefreshScreens);

    mainLayout->addWidget(headerFrame);

    connect(m_screenCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onScreenSelected);
    connect(m_btnIdentify, &QPushButton::clicked, this, &MainWindow::identifyCurrentScreen);
    connect(m_btnIdentifyAll, &QPushButton::clicked, this, &MainWindow::identifyAllScreens);
    connect(m_btnRefreshScreens, &QPushButton::clicked, this, &MainWindow::refreshScreenList);

    // 2. Main Tab Widget
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setObjectName("mainTabs");

    m_tabWidget->addTab(createWizardTab(), "🧙‍♂️ ガイド付きウィザード");
    m_tabWidget->addTab(createExplorerTab(), "🎨 パターン一覧・個別検査");
    m_tabWidget->addTab(createHardwareTab(), "⚙️ ディスプレイ & DDC/CI 設定");

    mainLayout->addWidget(m_tabWidget, 1);
}

QWidget* MainWindow::createWizardTab() {
    auto *tab = new QWidget(this);
    auto *layout = new QHBoxLayout(tab);
    layout->setContentsMargins(10, 14, 10, 10);
    layout->setSpacing(16);

    // Left Sidebar: Step List
    auto *leftPanel = new QFrame(this);
    leftPanel->setObjectName("cardPanel");
    leftPanel->setFixedWidth(240);
    auto *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(10, 12, 10, 12);
    leftLayout->setSpacing(8);

    auto *sidebarTitle = new QLabel("📋 調整ステップ", this);
    sidebarTitle->setStyleSheet("font-weight: bold; font-size: 13px; color: #94a3b8;");
    leftLayout->addWidget(sidebarTitle);

    m_stepList = new QListWidget(this);
    m_stepList->setObjectName("wizardStepList");
    const auto &steps = m_wizard->allSteps();
    for (int i = 0; i < steps.size(); ++i) {
        m_stepList->addItem(QString("%1. %2").arg(i + 1).arg(steps[i].title.split(' ')[0]));
    }
    m_stepList->setCurrentRow(0);
    leftLayout->addWidget(m_stepList, 1);

    connect(m_stepList, &QListWidget::currentRowChanged, this, [this](int row) {
        if (row >= 0 && row < m_wizard->totalSteps()) {
            m_wizard->goToStep(row);
        }
    });

    layout->addWidget(leftPanel);

    // Right Area: Details, Preview & Controls
    auto *rightPanel = new QFrame(this);
    rightPanel->setObjectName("cardPanel");
    auto *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(18, 16, 18, 16);
    rightLayout->setSpacing(12);

    // Header in Wizard Page
    m_wizardTitleLabel = new QLabel("黒レベル / 輝度の調整", this);
    m_wizardTitleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #60a5fa;");
    rightLayout->addWidget(m_wizardTitleLabel);

    m_wizardDescLabel = new QLabel(this);
    m_wizardDescLabel->setWordWrap(true);
    m_wizardDescLabel->setStyleSheet("font-size: 13px; color: #e2e8f0; line-height: 1.4;");
    rightLayout->addWidget(m_wizardDescLabel);

    // OSD Tip Box
    auto *tipFrame = new QFrame(this);
    tipFrame->setObjectName("tipFrame");
    auto *tipLayout = new QHBoxLayout(tipFrame);
    tipLayout->setContentsMargins(12, 8, 12, 8);
    m_wizardOsdTipLabel = new QLabel(this);
    m_wizardOsdTipLabel->setStyleSheet("color: #fde68a; font-size: 12px; font-weight: 500;");
    m_wizardOsdTipLabel->setWordWrap(true);
    tipLayout->addWidget(m_wizardOsdTipLabel);
    rightLayout->addWidget(tipFrame);

    // Pattern Preview Widget (Interactive live preview)
    m_previewPatternWidget = new PatternWidget(this);
    m_previewPatternWidget->setHudVisible(false); // In preview box, hide floating HUD
    m_previewPatternWidget->setMinimumHeight(240);
    m_previewPatternWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    rightLayout->addWidget(m_previewPatternWidget, 1);

    // Navigation & Fullscreen Buttons
    auto *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);

    m_btnWizardPrev = new QPushButton("◀ 前のステップ", this);
    btnLayout->addWidget(m_btnWizardPrev);

    m_btnWizardFullscreen = new QPushButton("🖥️ 全画面で調整 (推奨)", this);
    m_btnWizardFullscreen->setObjectName("primaryBtn");
    m_btnWizardFullscreen->setFixedHeight(40);
    m_btnWizardFullscreen->setStyleSheet("font-size: 14px; font-weight: bold;");
    btnLayout->addWidget(m_btnWizardFullscreen, 1);

    m_btnWizardNext = new QPushButton("次のステップ ▶", this);
    btnLayout->addWidget(m_btnWizardNext);

    rightLayout->addLayout(btnLayout);
    layout->addWidget(rightPanel, 1);

    connect(m_btnWizardPrev, &QPushButton::clicked, m_wizard, &WizardController::prevStep);
    connect(m_btnWizardNext, &QPushButton::clicked, m_wizard, &WizardController::nextStep);
    connect(m_btnWizardFullscreen, &QPushButton::clicked, this, [this]() {
        launchFullscreenPattern(m_wizard->currentStepInfo().patternType, true);
    });

    return tab;
}

QWidget* MainWindow::createExplorerTab() {
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto *container = new QWidget(scrollArea);
    auto *grid = new QGridLayout(container);
    grid->setContentsMargins(14, 14, 14, 14);
    grid->setSpacing(16);

    struct PatternCardData {
        PatternType type;
        QString title;
        QString desc;
        QString icon;
    };

    const QVector<PatternCardData> cards = {
        {PatternType::BlackLevel, "1. 黒レベル / 輝度 (Brightness)", "0%〜5%低輝度ステップと点滅ボックスによる暗部階調の基準調整", "🌑"},
        {PatternType::WhiteLevel, "2. 白レベル / コントラスト (Contrast)", "95%〜100%ハイライト階調とRGB階調による白飛び・クリッピング防止", "☀️"},
        {PatternType::Gamma22, "3. ガンマ 2.2 (Gamma 2.2)", "1px白黒ラスタラインと基準パッチ比較によるガンマ曲線調整", "📐"},
        {PatternType::GrayRamp, "4. グレースケール & カラーバランス", "32階調ステップバー、滑らかな連続グラデーション、RGB原色リニアリティ", "🌈"},
        {PatternType::Sharpness, "5. シャープネス & フォーカス", "1px白黒格子・市松模様による輪郭補正（ハロー/リンギング）の最適化", "🔍"},
        {PatternType::ColorUniformity, "6. 色均一性 & ドット抜け検査", "単色（白・灰・黒・赤・緑・青）全画面表示によるムラ・常時点灯ドット検査", "🎨"},
        {PatternType::GeometryFocus, "7. 画面比率 & オーバースキャン", "1:1ピクセルマッピング、ドット・バイ・ドット、外枠1px欠けの検査", "📏"}
    };

    int row = 0;
    int col = 0;
    for (const auto &card : cards) {
        auto *frame = new QFrame(container);
        frame->setObjectName("cardPanel");
        auto *cardLayout = new QVBoxLayout(frame);
        cardLayout->setContentsMargins(14, 14, 14, 14);
        cardLayout->setSpacing(8);

        auto *title = new QLabel(QString("%1 %2").arg(card.icon, card.title), frame);
        title->setStyleSheet("font-size: 14px; font-weight: bold; color: #60a5fa;");
        cardLayout->addWidget(title);

        auto *desc = new QLabel(card.desc, frame);
        desc->setWordWrap(true);
        desc->setStyleSheet("color: #cbd5e1; font-size: 11px;");
        cardLayout->addWidget(desc, 1);

        auto *btn = new QPushButton("全画面で表示", frame);
        btn->setStyleSheet("background-color: #334155; color: white; padding: 6px; font-weight: bold; border-radius: 6px;");
        PatternType pType = card.type;
        connect(btn, &QPushButton::clicked, this, [this, pType]() {
            launchFullscreenPattern(pType, false);
        });
        cardLayout->addWidget(btn);

        grid->addWidget(frame, row, col);
        col++;
        if (col >= 2) {
            col = 0;
            row++;
        }
    }

    scrollArea->setWidget(container);
    return scrollArea;
}

QWidget* MainWindow::createHardwareTab() {
    auto *tab = new QWidget(this);
    auto *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(16);

    // Screen Info Box
    auto *infoGroup = new QGroupBox("🖥️ ディスプレイ情報", tab);
    auto *infoGrid = new QGridLayout(infoGroup);
    infoGrid->setContentsMargins(16, 16, 16, 16);
    infoGrid->setHorizontalSpacing(24);
    infoGrid->setVerticalSpacing(10);

    infoGrid->addWidget(new QLabel("モニタ名 / 型番:", infoGroup), 0, 0);
    m_lblScreenModel = new QLabel("-", infoGroup);
    m_lblScreenModel->setStyleSheet("font-weight: bold; color: #60a5fa;");
    infoGrid->addWidget(m_lblScreenModel, 0, 1);

    infoGrid->addWidget(new QLabel("解像度 / 位置:", infoGroup), 1, 0);
    m_lblScreenRes = new QLabel("-", infoGroup);
    infoGrid->addWidget(m_lblScreenRes, 1, 1);

    infoGrid->addWidget(new QLabel("リフレッシュレート:", infoGroup), 2, 0);
    m_lblScreenRate = new QLabel("-", infoGroup);
    infoGrid->addWidget(m_lblScreenRate, 2, 1);

    infoGrid->addWidget(new QLabel("DPI / スケール:", infoGroup), 3, 0);
    m_lblScreenDpi = new QLabel("-", infoGroup);
    infoGrid->addWidget(m_lblScreenDpi, 3, 1);

    layout->addWidget(infoGroup);

    // Hardware Control Box
    auto *hwGroup = new QGroupBox("🎛️ ハードウェア / ソフトウェア制御 (DDC/CI & XRandR)", tab);
    auto *hwLayout = new QVBoxLayout(hwGroup);
    hwLayout->setContentsMargins(16, 16, 16, 16);
    hwLayout->setSpacing(14);

    m_lblDdcStatus = new QLabel("ステータス: 確認中...", hwGroup);
    m_lblDdcStatus->setStyleSheet("color: #94a3b8;");
    hwLayout->addWidget(m_lblDdcStatus);

    // Brightness slider
    auto *brightLayout = new QHBoxLayout();
    brightLayout->addWidget(new QLabel("輝度 (Brightness):", hwGroup));
    m_sliderBrightness = new QSlider(Qt::Horizontal, hwGroup);
    m_sliderBrightness->setRange(0, 100);
    m_sliderBrightness->setValue(50);
    brightLayout->addWidget(m_sliderBrightness, 1);
    m_lblBrightnessVal = new QLabel("50%", hwGroup);
    m_lblBrightnessVal->setFixedWidth(45);
    brightLayout->addWidget(m_lblBrightnessVal);
    hwLayout->addLayout(brightLayout);

    // Contrast slider
    auto *contrastLayout = new QHBoxLayout();
    contrastLayout->addWidget(new QLabel("コントラスト (Contrast):", hwGroup));
    m_sliderContrast = new QSlider(Qt::Horizontal, hwGroup);
    m_sliderContrast->setRange(0, 100);
    m_sliderContrast->setValue(50);
    contrastLayout->addWidget(m_sliderContrast, 1);
    m_lblContrastVal = new QLabel("50%", hwGroup);
    m_lblContrastVal->setFixedWidth(45);
    contrastLayout->addWidget(m_lblContrastVal);
    hwLayout->addLayout(contrastLayout);

    connect(m_sliderBrightness, &QSlider::valueChanged, this, &MainWindow::onBrightnessSliderMoved);
    connect(m_sliderContrast, &QSlider::valueChanged, this, &MainWindow::onContrastSliderMoved);

    m_btnResetSoftware = new QPushButton("ソフトウェア補正 (ガンマ/輝度) をデフォルトにリセット", hwGroup);
    connect(m_btnResetSoftware, &QPushButton::clicked, this, [this]() {
        int idx = m_screenCombo->currentIndex();
        auto screens = m_screenManager->getScreenList();
        QString name = (idx >= 0 && idx < screens.size()) ? screens[idx].name : QString();
        m_hardwareBridge->resetSoftwareSettings(name);
        QMessageBox::information(this, "リセット完了", "XRandR ソフトウェア設定をデフォルト (1.0) に戻しました。");
    });
    hwLayout->addWidget(m_btnResetSoftware);

    layout->addWidget(hwGroup);
    layout->addStretch(1);

    return tab;
}

void MainWindow::applyTheme() {
    setStyleSheet(
        "QMainWindow, QWidget {"
        "  background-color: #0f172a;" // Slate 900
        "  color: #f8fafc;"
        "  font-family: 'Noto Sans CJK JP', 'Noto Sans', 'Segoe UI', sans-serif;"
        "}"
        "#headerFrame {"
        "  background-color: #1e293b;" // Slate 800
        "  border-radius: 10px;"
        "  border: 1px solid #334155;"
        "}"
        "#cardPanel {"
        "  background-color: #1e293b;"
        "  border-radius: 12px;"
        "  border: 1px solid #334155;"
        "}"
        "#tipFrame {"
        "  background-color: #451a03;"
        "  border-radius: 8px;"
        "  border: 1px solid #b45309;"
        "}"
        "QTabWidget::pane {"
        "  border: 1px solid #334155;"
        "  border-radius: 10px;"
        "  background-color: #1e293b;"
        "  top: -1px;"
        "}"
        "QTabBar::tab {"
        "  background: #0f172a;"
        "  color: #94a3b8;"
        "  padding: 10px 18px;"
        "  border: 1px solid #334155;"
        "  border-bottom: none;"
        "  border-top-left-radius: 8px;"
        "  border-top-right-radius: 8px;"
        "  margin-right: 4px;"
        "  font-weight: bold;"
        "}"
        "QTabBar::tab:selected {"
        "  background: #1e293b;"
        "  color: #60a5fa;"
        "  border-bottom: 2px solid #3b82f6;"
        "}"
        "QListWidget {"
        "  background-color: #0f172a;"
        "  border: 1px solid #334155;"
        "  border-radius: 8px;"
        "  padding: 4px;"
        "}"
        "QListWidget::item {"
        "  padding: 10px 8px;"
        "  border-radius: 6px;"
        "  color: #cbd5e1;"
        "  margin-bottom: 2px;"
        "}"
        "QListWidget::item:selected {"
        "  background-color: #2563eb;"
        "  color: #ffffff;"
        "  font-weight: bold;"
        "}"
        "QPushButton {"
        "  background-color: #334155;"
        "  color: #f8fafc;"
        "  border: 1px solid #475569;"
        "  border-radius: 6px;"
        "  padding: 8px 14px;"
        "  font-weight: 500;"
        "}"
        "QPushButton:hover {"
        "  background-color: #475569;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #1e293b;"
        "}"
        "#primaryBtn {"
        "  background-color: #2563eb;"
        "  border: 1px solid #3b82f6;"
        "  color: white;"
        "}"
        "#primaryBtn:hover {"
        "  background-color: #1d4ed8;"
        "}"
        "QComboBox {"
        "  background-color: #0f172a;"
        "  border: 1px solid #475569;"
        "  border-radius: 6px;"
        "  padding: 6px 12px;"
        "  color: #f8fafc;"
        "}"
        "QGroupBox {"
        "  border: 1px solid #334155;"
        "  border-radius: 8px;"
        "  margin-top: 10px;"
        "  padding-top: 14px;"
        "  font-weight: bold;"
        "  color: #94a3b8;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  subcontrol-position: top left;"
        "  left: 12px;"
        "  padding: 0 4px;"
        "}"
        "QSlider::groove:horizontal {"
        "  border: none;"
        "  height: 6px;"
        "  background: #334155;"
        "  border-radius: 3px;"
        "}"
        "QSlider::sub-page:horizontal {"
        "  background: #3b82f6;"
        "  border-radius: 3px;"
        "}"
        "QSlider::handle:horizontal {"
        "  background: #60a5fa;"
        "  width: 16px;"
        "  margin-top: -5px;"
        "  margin-bottom: -5px;"
        "  border-radius: 8px;"
        "}"
    );
}

void MainWindow::refreshScreenList() {
    m_screenCombo->blockSignals(true);
    m_screenCombo->clear();

    const auto screens = m_screenManager->getScreenList();
    for (const auto &s : screens) {
        QString text = QString("🖥️ %1: %2x%3 @ %4Hz %5")
                           .arg(s.name)
                           .arg(s.geometry.width())
                           .arg(s.geometry.height())
                           .arg(s.refreshRate)
                           .arg(s.isPrimary ? "(プライマリ)" : "");
        m_screenCombo->addItem(text, s.index);
    }

    m_screenCombo->blockSignals(false);
    if (!screens.isEmpty()) {
        m_screenCombo->setCurrentIndex(m_screenManager->getPrimaryScreenIndex());
    }

    updateScreenDetails();
}

void MainWindow::onScreenSelected(int index) {
    Q_UNUSED(index);
    updateScreenDetails();
}

void MainWindow::updateScreenDetails() {
    int idx = m_screenCombo->currentIndex();
    auto screens = m_screenManager->getScreenList();
    if (idx < 0 || idx >= screens.size()) return;

    const auto &s = screens[idx];
    m_lblScreenModel->setText(QString("%1 %2").arg(s.manufacturer, s.model).trimmed());
    m_lblScreenRes->setText(QString("%1 x %2 (位置: X=%3, Y=%4)")
                                .arg(s.geometry.width())
                                .arg(s.geometry.height())
                                .arg(s.geometry.x())
                                .arg(s.geometry.y()));
    m_lblScreenRate->setText(QString("%1 Hz (色深度: %2 bit)").arg(s.refreshRate).arg(s.depth));
    m_lblScreenDpi->setText(QString("DPI: %1 (スケール: %2x)").arg(qRound(s.logicalDotsPerInch)).arg(s.devicePixelRatio));

    m_hardwareBridge->probeCapabilities(s.name);
}

void MainWindow::onHardwareCapsUpdated(const HardwareCapabilities &caps) {
    if (caps.hasDdcUtil && caps.ddcResponsive) {
        m_lblDdcStatus->setText("✅ DDC/CI 連携可能 (モニタ内部パラメータを直接制御できます)");
        m_lblDdcStatus->setStyleSheet("color: #4ade80; font-weight: bold;");
        m_sliderBrightness->setEnabled(true);
        m_sliderContrast->setEnabled(true);
        if (caps.currentBrightness >= 0) {
            m_sliderBrightness->setValue(caps.currentBrightness);
            m_lblBrightnessVal->setText(QString("%1%").arg(caps.currentBrightness));
        }
        if (caps.currentContrast >= 0) {
            m_sliderContrast->setValue(caps.currentContrast);
            m_lblContrastVal->setText(QString("%1%").arg(caps.currentContrast));
        }
    } else if (caps.hasXrandr) {
        m_lblDdcStatus->setText("⚠️ DDC/CI不可 (XRandR ソフトウェア補正またはモニタOSDボタンを使用してください)");
        m_lblDdcStatus->setStyleSheet("color: #fbbf24; font-weight: bold;");
        m_sliderBrightness->setEnabled(true);
        m_sliderContrast->setEnabled(false);
    } else {
        m_lblDdcStatus->setText("ℹ️ モニタOSD手動調整モード (モニタ本体の操作ボタンで調整してください)");
        m_lblDdcStatus->setStyleSheet("color: #94a3b8;");
        m_sliderBrightness->setEnabled(false);
        m_sliderContrast->setEnabled(false);
    }
}

void MainWindow::onBrightnessSliderMoved(int val) {
    m_lblBrightnessVal->setText(QString("%1%").arg(val));
    int idx = m_screenCombo->currentIndex();
    auto screens = m_screenManager->getScreenList();
    QString name = (idx >= 0 && idx < screens.size()) ? screens[idx].name : QString();
    m_hardwareBridge->setBrightness(val, name);
}

void MainWindow::onContrastSliderMoved(int val) {
    m_lblContrastVal->setText(QString("%1%").arg(val));
    m_hardwareBridge->setContrast(val);
}

void MainWindow::identifyCurrentScreen() {
    int idx = m_screenCombo->currentIndex();
    m_screenManager->showIdentifyOverlay(idx);
}

void MainWindow::identifyAllScreens() {
    m_screenManager->showIdentifyOverlayAll();
}

void MainWindow::onWizardStepChanged(int stepIndex, const WizardStepInfo &info) {
    if (m_stepList->currentRow() != stepIndex) {
        m_stepList->setCurrentRow(stepIndex);
    }

    m_wizardTitleLabel->setText(info.title);
    m_wizardDescLabel->setText(info.instruction);
    m_wizardOsdTipLabel->setText(info.osdTip);

    m_previewPatternWidget->setPatternType(info.patternType);
    m_previewPatternWidget->setUniformityColor(info.defaultUniformityColor);

    m_btnWizardPrev->setEnabled(stepIndex > 0);
    m_btnWizardNext->setText(stepIndex == m_wizard->totalSteps() - 1 ? "完了 🎉" : "次のステップ ▶");

    if (m_fullscreenWindow && m_fullscreenPatternWidget && m_isFullscreenWizard) {
        m_fullscreenPatternWidget->setPatternType(info.patternType);
        m_fullscreenPatternWidget->setUniformityColor(info.defaultUniformityColor);
        m_fullscreenPatternWidget->setGuideInfo(info.title, info.instruction, info.osdTip);
        m_fullscreenPatternWidget->setStepNavigation(stepIndex, m_wizard->totalSteps());
    }
}

void MainWindow::launchFullscreenPattern(PatternType type, bool inWizardMode) {
    m_isFullscreenWizard = inWizardMode;

    if (!m_fullscreenWindow) {
        m_fullscreenWindow = new QWidget(nullptr, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        auto *layout = new QVBoxLayout(m_fullscreenWindow);
        layout->setContentsMargins(0, 0, 0, 0);

        m_fullscreenPatternWidget = new PatternWidget(m_fullscreenWindow);
        layout->addWidget(m_fullscreenPatternWidget);

        connect(m_fullscreenPatternWidget, &PatternWidget::closeRequested, this, &MainWindow::closeFullscreenPattern);
        connect(m_fullscreenPatternWidget, &PatternWidget::nextStepRequested, this, [this]() {
            if (m_isFullscreenWizard) m_wizard->nextStep();
        });
        connect(m_fullscreenPatternWidget, &PatternWidget::prevStepRequested, this, [this]() {
            if (m_isFullscreenWizard) m_wizard->prevStep();
        });
        connect(m_fullscreenPatternWidget, &PatternWidget::toggleFullscreenRequested, this, [this]() {
            if (m_fullscreenWindow->isFullScreen()) {
                m_fullscreenWindow->showNormal();
            } else {
                m_fullscreenWindow->showFullScreen();
            }
        });
    }

    int screenIdx = m_screenCombo->currentIndex();
    QScreen *targetScreen = m_screenManager->getScreen(screenIdx);
    if (targetScreen) {
        m_fullscreenWindow->setGeometry(targetScreen->geometry());
    }

    if (inWizardMode) {
        const auto &info = m_wizard->currentStepInfo();
        m_fullscreenPatternWidget->setPatternType(info.patternType);
        m_fullscreenPatternWidget->setUniformityColor(info.defaultUniformityColor);
        m_fullscreenPatternWidget->setGuideInfo(info.title, info.instruction, info.osdTip);
        m_fullscreenPatternWidget->setStepNavigation(m_wizard->currentStepIndex(), m_wizard->totalSteps());
    } else {
        m_fullscreenPatternWidget->setPatternType(type);
        m_fullscreenPatternWidget->setGuideInfo("テストパターン表示中", "キーボードの [Esc] で終了、[F] で全画面/ウィンドウ切り替え", "");
        m_fullscreenPatternWidget->setStepNavigation(-1, -1);
    }

    m_fullscreenWindow->showFullScreen();
    m_fullscreenPatternWidget->setFocus();
}

void MainWindow::closeFullscreenPattern() {
    if (m_fullscreenWindow) {
        m_fullscreenWindow->close();
    }
    show();
    activateWindow();
    raise();
}
