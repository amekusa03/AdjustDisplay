#include "main_window.h"
#include "i18n.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QMessageBox>
#include <QApplication>
#include <QScreen>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_screenManager(new ScreenManager(this))
    , m_hardwareBridge(new HardwareBridge(this))
    , m_wizard(new WizardController(this))
{
    setupUi();
    applyTheme();

    connect(m_screenManager, &ScreenManager::screensChanged, this, &MainWindow::refreshScreenList);
    connect(m_hardwareBridge, &HardwareBridge::capabilitiesUpdated, this, &MainWindow::onHardwareCapsUpdated);
    connect(m_wizard, &WizardController::stepChanged, this, &MainWindow::onWizardStepChanged);
    connect(m_wizard, &WizardController::wizardFinished, this, [this]() {
        if (m_fullscreenWindow && m_fullscreenWindow->isVisible()) {
            closeFullscreenPattern();
        }
    });

    connect(I18n::instance(), &I18n::languageChanged, this, &MainWindow::retranslateUi);

    refreshScreenList();
    m_wizard->reset();
    retranslateUi();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi() {
    setMinimumSize(960, 680);
    resize(1020, 720);

    auto *central = new QWidget(this);
    setCentralWidget(central);

    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    // 1. Top Header Bar: Screen Selector & Action Buttons & Language Selector
    auto *headerFrame = new QFrame(this);
    headerFrame->setObjectName("headerFrame");
    auto *headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setContentsMargins(14, 10, 14, 10);
    headerLayout->setSpacing(10);

    m_screenCombo = new QComboBox(this);
    m_screenCombo->setMinimumWidth(240);
    headerLayout->addWidget(m_screenCombo, 1);

    m_btnIdentify = new QPushButton(this);
    headerLayout->addWidget(m_btnIdentify);

    m_btnIdentifyAll = new QPushButton(this);
    headerLayout->addWidget(m_btnIdentifyAll);

    m_btnRefreshScreens = new QPushButton(this);
    headerLayout->addWidget(m_btnRefreshScreens);

    // Language Selector
    m_langCombo = new QComboBox(this);
    m_langCombo->setFixedWidth(120);
    m_langCombo->addItem("English", static_cast<int>(Language::English));
    m_langCombo->addItem("日本語", static_cast<int>(Language::Japanese));

    int currentLangIdx = (I18n::instance()->language() == Language::Japanese) ? 1 : 0;
    m_langCombo->setCurrentIndex(currentLangIdx);
    headerLayout->addWidget(m_langCombo);

    mainLayout->addWidget(headerFrame);

    connect(m_screenCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onScreenSelected);
    connect(m_btnIdentify, &QPushButton::clicked, this, &MainWindow::identifyCurrentScreen);
    connect(m_btnIdentifyAll, &QPushButton::clicked, this, &MainWindow::identifyAllScreens);
    connect(m_btnRefreshScreens, &QPushButton::clicked, this, &MainWindow::refreshScreenList);
    connect(m_langCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onLanguageComboChanged);

    // 2. Main Tab Widget
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setObjectName("mainTabs");

    m_tabWidget->addTab(createWizardTab(), "");
    m_tabWidget->addTab(createExplorerTab(), "");
    m_tabWidget->addTab(createHardwareTab(), "");

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

    m_lblWizardSidebarTitle = new QLabel(this);
    m_lblWizardSidebarTitle->setStyleSheet("font-weight: bold; font-size: 13px; color: #94a3b8;");
    leftLayout->addWidget(m_lblWizardSidebarTitle);

    m_stepList = new QListWidget(this);
    m_stepList->setObjectName("wizardStepList");
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
    m_wizardTitleLabel = new QLabel(this);
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

    m_btnWizardPrev = new QPushButton(this);
    btnLayout->addWidget(m_btnWizardPrev);

    m_btnWizardFullscreen = new QPushButton(this);
    m_btnWizardFullscreen->setObjectName("primaryBtn");
    m_btnWizardFullscreen->setFixedHeight(40);
    m_btnWizardFullscreen->setStyleSheet("font-size: 14px; font-weight: bold;");
    btnLayout->addWidget(m_btnWizardFullscreen, 1);

    m_btnWizardNext = new QPushButton(this);
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
        const char *titleKey;
        const char *descKey;
        QString icon;
    };

    const QVector<PatternCardData> cards = {
        {PatternType::BlackLevel, "card_black_title", "card_black_desc", "🌑"},
        {PatternType::WhiteLevel, "card_white_title", "card_white_desc", "☀️"},
        {PatternType::Gamma22, "card_gamma_title", "card_gamma_desc", "📐"},
        {PatternType::GrayRamp, "card_gray_title", "card_gray_desc", "🌈"},
        {PatternType::Sharpness, "card_sharp_title", "card_sharp_desc", "🔍"},
        {PatternType::ColorUniformity, "card_unif_title", "card_unif_desc", "🎨"},
        {PatternType::GeometryFocus, "card_geom_title", "card_geom_desc", "📏"}
    };

    m_explorerCards.clear();
    int row = 0;
    int col = 0;
    for (const auto &card : cards) {
        auto *frame = new QFrame(container);
        frame->setObjectName("cardPanel");
        auto *cardLayout = new QVBoxLayout(frame);
        cardLayout->setContentsMargins(14, 14, 14, 14);
        cardLayout->setSpacing(8);

        auto *title = new QLabel(frame);
        title->setStyleSheet("font-size: 14px; font-weight: bold; color: #60a5fa;");
        cardLayout->addWidget(title);

        auto *desc = new QLabel(frame);
        desc->setWordWrap(true);
        desc->setStyleSheet("color: #cbd5e1; font-size: 11px;");
        cardLayout->addWidget(desc, 1);

        auto *btn = new QPushButton(frame);
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

        m_explorerCards.append({card.type, title, desc, btn});
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
    m_groupScreenInfo = new QGroupBox(tab);
    auto *infoGrid = new QGridLayout(m_groupScreenInfo);
    infoGrid->setContentsMargins(16, 16, 16, 16);
    infoGrid->setHorizontalSpacing(24);
    infoGrid->setVerticalSpacing(10);

    m_lblScreenModelHeader = new QLabel(m_groupScreenInfo);
    infoGrid->addWidget(m_lblScreenModelHeader, 0, 0);
    m_lblScreenModel = new QLabel("-", m_groupScreenInfo);
    m_lblScreenModel->setStyleSheet("font-weight: bold; color: #60a5fa;");
    infoGrid->addWidget(m_lblScreenModel, 0, 1);

    m_lblScreenResHeader = new QLabel(m_groupScreenInfo);
    infoGrid->addWidget(m_lblScreenResHeader, 1, 0);
    m_lblScreenRes = new QLabel("-", m_groupScreenInfo);
    infoGrid->addWidget(m_lblScreenRes, 1, 1);

    m_lblScreenRateHeader = new QLabel(m_groupScreenInfo);
    infoGrid->addWidget(m_lblScreenRateHeader, 2, 0);
    m_lblScreenRate = new QLabel("-", m_groupScreenInfo);
    infoGrid->addWidget(m_lblScreenRate, 2, 1);

    m_lblScreenDpiHeader = new QLabel(m_groupScreenInfo);
    infoGrid->addWidget(m_lblScreenDpiHeader, 3, 0);
    m_lblScreenDpi = new QLabel("-", m_groupScreenInfo);
    infoGrid->addWidget(m_lblScreenDpi, 3, 1);

    layout->addWidget(m_groupScreenInfo);

    // Hardware Control Box
    m_groupHwCtrl = new QGroupBox(tab);
    auto *hwLayout = new QVBoxLayout(m_groupHwCtrl);
    hwLayout->setContentsMargins(16, 16, 16, 16);
    hwLayout->setSpacing(14);

    m_lblDdcStatus = new QLabel(m_groupHwCtrl);
    m_lblDdcStatus->setStyleSheet("color: #94a3b8;");
    hwLayout->addWidget(m_lblDdcStatus);

    // Brightness slider
    auto *brightLayout = new QHBoxLayout();
    m_lblBrightnessHeader = new QLabel(m_groupHwCtrl);
    brightLayout->addWidget(m_lblBrightnessHeader);
    m_sliderBrightness = new QSlider(Qt::Horizontal, m_groupHwCtrl);
    m_sliderBrightness->setRange(0, 100);
    m_sliderBrightness->setValue(50);
    brightLayout->addWidget(m_sliderBrightness, 1);
    m_lblBrightnessVal = new QLabel("50%", m_groupHwCtrl);
    m_lblBrightnessVal->setFixedWidth(45);
    brightLayout->addWidget(m_lblBrightnessVal);
    hwLayout->addLayout(brightLayout);

    // Contrast slider
    auto *contrastLayout = new QHBoxLayout();
    m_lblContrastHeader = new QLabel(m_groupHwCtrl);
    contrastLayout->addWidget(m_lblContrastHeader);
    m_sliderContrast = new QSlider(Qt::Horizontal, m_groupHwCtrl);
    m_sliderContrast->setRange(0, 100);
    m_sliderContrast->setValue(50);
    contrastLayout->addWidget(m_sliderContrast, 1);
    m_lblContrastVal = new QLabel("50%", m_groupHwCtrl);
    m_lblContrastVal->setFixedWidth(45);
    contrastLayout->addWidget(m_lblContrastVal);
    hwLayout->addLayout(contrastLayout);

    connect(m_sliderBrightness, &QSlider::valueChanged, this, &MainWindow::onBrightnessSliderMoved);
    connect(m_sliderContrast, &QSlider::valueChanged, this, &MainWindow::onContrastSliderMoved);

    m_btnResetSoftware = new QPushButton(m_groupHwCtrl);
    connect(m_btnResetSoftware, &QPushButton::clicked, this, [this]() {
        auto *i18n = I18n::instance();
        int idx = m_screenCombo->currentIndex();
        auto screens = m_screenManager->getScreenList();
        QString name = (idx >= 0 && idx < screens.size()) ? screens[idx].name : QString();
        m_hardwareBridge->resetSoftwareSettings(name);
        QMessageBox::information(this, i18n->t("hw_dlg_reset_title"), i18n->t("hw_dlg_reset_msg"));
    });
    hwLayout->addWidget(m_btnResetSoftware);

    layout->addWidget(m_groupHwCtrl);
    layout->addStretch(1);

    return tab;
}

void MainWindow::onLanguageComboChanged(int index) {
    Language lang = static_cast<Language>(m_langCombo->itemData(index).toInt());
    I18n::instance()->setLanguage(lang);
}

void MainWindow::retranslateUi() {
    auto *i18n = I18n::instance();

    // Window Title & App
    setWindowTitle(i18n->t("app_title"));

    // Header buttons
    m_btnIdentify->setText(i18n->t("btn_identify"));
    m_btnIdentify->setToolTip(i18n->t("btn_identify_tip"));
    m_btnIdentifyAll->setText(i18n->t("btn_identify_all"));
    m_btnIdentifyAll->setToolTip(i18n->t("btn_identify_all_tip"));
    m_btnRefreshScreens->setText(i18n->t("btn_refresh"));

    // Language combo sync
    int targetIdx = (i18n->language() == Language::Japanese) ? 1 : 0;
    if (m_langCombo->currentIndex() != targetIdx) {
        m_langCombo->blockSignals(true);
        m_langCombo->setCurrentIndex(targetIdx);
        m_langCombo->blockSignals(false);
    }

    // Tabs
    m_tabWidget->setTabText(0, i18n->t("tab_wizard"));
    m_tabWidget->setTabText(1, i18n->t("tab_explorer"));
    m_tabWidget->setTabText(2, i18n->t("tab_hardware"));

    // Wizard Tab
    m_lblWizardSidebarTitle->setText(i18n->t("wizard_sidebar_title"));
    m_btnWizardPrev->setText(i18n->t("btn_wizard_prev"));
    m_btnWizardFullscreen->setText(i18n->t("btn_wizard_fullscreen"));

    // Step List
    int curRow = m_stepList->currentRow();
    m_stepList->blockSignals(true);
    m_stepList->clear();
    const char* stepKeys[] = {
        "step1_short", "step2_short", "step3_short",
        "step4_short", "step5_short", "step6_short", "step7_short"
    };
    for (int i = 0; i < 7; ++i) {
        m_stepList->addItem(i18n->t(stepKeys[i]));
    }
    if (curRow >= 0 && curRow < m_stepList->count()) {
        m_stepList->setCurrentRow(curRow);
    } else {
        m_stepList->setCurrentRow(0);
    }
    m_stepList->blockSignals(false);

    // Update wizard current step labels
    const auto &curInfo = m_wizard->currentStepInfo();
    onWizardStepChanged(m_wizard->currentStepIndex(), curInfo);

    // Explorer cards
    const char* cardTitles[] = {
        "card_black_title", "card_white_title", "card_gamma_title",
        "card_gray_title", "card_sharp_title", "card_unif_title", "card_geom_title"
    };
    const char* cardDescs[] = {
        "card_black_desc", "card_white_desc", "card_gamma_desc",
        "card_gray_desc", "card_sharp_desc", "card_unif_desc", "card_geom_desc"
    };
    const QString icons[] = {"🌑", "☀️", "📐", "🌈", "🔍", "🎨", "📏"};

    for (int i = 0; i < m_explorerCards.size() && i < 7; ++i) {
        m_explorerCards[i].titleLabel->setText(QString("%1 %2").arg(icons[i], i18n->t(cardTitles[i])));
        m_explorerCards[i].descLabel->setText(i18n->t(cardDescs[i]));
        m_explorerCards[i].actionBtn->setText(i18n->t("btn_open_fullscreen"));
    }

    // Hardware Tab
    m_groupScreenInfo->setTitle(i18n->t("hw_info_group"));
    m_lblScreenModelHeader->setText(i18n->t("hw_lbl_model"));
    m_lblScreenResHeader->setText(i18n->t("hw_lbl_res"));
    m_lblScreenRateHeader->setText(i18n->t("hw_lbl_rate"));
    m_lblScreenDpiHeader->setText(i18n->t("hw_lbl_dpi"));

    m_groupHwCtrl->setTitle(i18n->t("hw_ctrl_group"));
    m_lblBrightnessHeader->setText(i18n->t("hw_lbl_brightness"));
    m_lblContrastHeader->setText(i18n->t("hw_lbl_contrast"));
    m_btnResetSoftware->setText(i18n->t("hw_btn_reset_sw"));

    // Refresh dynamic info
    updateScreenDetails();
    onHardwareCapsUpdated(m_hardwareBridge->capabilities());
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
    auto *i18n = I18n::instance();
    int currentSelectedIdx = m_screenCombo->currentIndex();

    m_screenCombo->blockSignals(true);
    m_screenCombo->clear();

    const auto screens = m_screenManager->getScreenList();
    for (const auto &s : screens) {
        QString text = QString("🖥️ %1: %2x%3 @ %4Hz %5")
                           .arg(s.name)
                           .arg(s.geometry.width())
                           .arg(s.geometry.height())
                           .arg(s.refreshRate)
                           .arg(s.isPrimary ? i18n->t("screen_primary_tag") : "");
        m_screenCombo->addItem(text, s.index);
    }

    m_screenCombo->blockSignals(false);
    if (!screens.isEmpty()) {
        if (currentSelectedIdx >= 0 && currentSelectedIdx < screens.size()) {
            m_screenCombo->setCurrentIndex(currentSelectedIdx);
        } else {
            m_screenCombo->setCurrentIndex(m_screenManager->getPrimaryScreenIndex());
        }
    }

    updateScreenDetails();
}

void MainWindow::onScreenSelected(int index) {
    Q_UNUSED(index);
    updateScreenDetails();
}

void MainWindow::updateScreenDetails() {
    auto *i18n = I18n::instance();
    int idx = m_screenCombo->currentIndex();
    auto screens = m_screenManager->getScreenList();
    if (idx < 0 || idx >= screens.size()) return;

    const auto &s = screens[idx];
    m_lblScreenModel->setText(QString("%1 %2").arg(s.manufacturer, s.model).trimmed());
    m_lblScreenRes->setText(i18n->t("hw_val_res")
                                .arg(s.geometry.width())
                                .arg(s.geometry.height())
                                .arg(s.geometry.x())
                                .arg(s.geometry.y()));
    m_lblScreenRate->setText(i18n->t("hw_val_rate").arg(s.refreshRate).arg(s.depth));
    m_lblScreenDpi->setText(i18n->t("hw_val_dpi").arg(qRound(s.logicalDotsPerInch)).arg(s.devicePixelRatio));

    m_hardwareBridge->probeCapabilities(s.name);
}

void MainWindow::onHardwareCapsUpdated(const HardwareCapabilities &caps) {
    auto *i18n = I18n::instance();
    if (caps.hasDdcUtil && caps.ddcResponsive) {
        m_lblDdcStatus->setText(i18n->t("hw_status_ddc_ok"));
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
        m_lblDdcStatus->setText(i18n->t("hw_status_xrandr"));
        m_lblDdcStatus->setStyleSheet("color: #fbbf24; font-weight: bold;");
        m_sliderBrightness->setEnabled(true);
        m_sliderContrast->setEnabled(false);
    } else {
        m_lblDdcStatus->setText(i18n->t("hw_status_manual"));
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
    auto *i18n = I18n::instance();
    if (m_stepList->currentRow() != stepIndex) {
        m_stepList->setCurrentRow(stepIndex);
    }

    m_wizardTitleLabel->setText(info.title);
    m_wizardDescLabel->setText(info.instruction);
    m_wizardOsdTipLabel->setText(info.osdTip);

    m_previewPatternWidget->setPatternType(info.patternType);
    m_previewPatternWidget->setUniformityColor(info.defaultUniformityColor);

    m_btnWizardPrev->setEnabled(stepIndex > 0);
    m_btnWizardNext->setText(stepIndex == m_wizard->totalSteps() - 1 ? i18n->t("btn_wizard_finish") : i18n->t("btn_wizard_next"));

    if (m_fullscreenWindow && m_fullscreenPatternWidget && m_isFullscreenWizard) {
        m_fullscreenPatternWidget->setPatternType(info.patternType);
        m_fullscreenPatternWidget->setUniformityColor(info.defaultUniformityColor);
        m_fullscreenPatternWidget->setGuideInfo(info.title, info.instruction, info.osdTip);
        m_fullscreenPatternWidget->setStepNavigation(stepIndex, m_wizard->totalSteps());
    }
}

void MainWindow::launchFullscreenPattern(PatternType type, bool inWizardMode) {
    auto *i18n = I18n::instance();
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
        m_fullscreenPatternWidget->setGuideInfo(i18n->t("hud_single_title"), i18n->t("hud_single_desc"), "");
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
