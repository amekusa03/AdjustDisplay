#pragma once

#include <QMainWindow>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QTabWidget>
#include <QListWidget>
#include <QGroupBox>
#include <QScrollArea>
#include "screen_manager.h"
#include "hardware_bridge.h"
#include "wizard_controller.h"
#include "pattern_widget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void refreshScreenList();
    void onScreenSelected(int index);
    void identifyCurrentScreen();
    void identifyAllScreens();

    void onWizardStepChanged(int stepIndex, const WizardStepInfo &info);
    void launchFullscreenPattern(PatternType type, bool inWizardMode = true);
    void closeFullscreenPattern();

    void onHardwareCapsUpdated(const HardwareCapabilities &caps);
    void onBrightnessSliderMoved(int val);
    void onContrastSliderMoved(int val);

    void onLanguageComboChanged(int index);
    void retranslateUi();

private:
    void setupUi();
    void applyTheme();
    QWidget* createWizardTab();
    QWidget* createExplorerTab();
    QWidget* createHardwareTab();
    void updateScreenDetails();
    void updateHardwareUi();

    ScreenManager *m_screenManager = nullptr;
    HardwareBridge *m_hardwareBridge = nullptr;
    WizardController *m_wizard = nullptr;

    // Header UI Widgets
    QComboBox *m_screenCombo = nullptr;
    QPushButton *m_btnIdentify = nullptr;
    QPushButton *m_btnIdentifyAll = nullptr;
    QPushButton *m_btnRefreshScreens = nullptr;
    QComboBox *m_langCombo = nullptr;

    QTabWidget *m_tabWidget = nullptr;

    // Wizard Tab UI
    QLabel *m_lblWizardSidebarTitle = nullptr;
    QListWidget *m_stepList = nullptr;
    QLabel *m_wizardTitleLabel = nullptr;
    QLabel *m_wizardDescLabel = nullptr;
    QLabel *m_wizardOsdTipLabel = nullptr;
    PatternWidget *m_previewPatternWidget = nullptr;
    QPushButton *m_btnWizardPrev = nullptr;
    QPushButton *m_btnWizardNext = nullptr;
    QPushButton *m_btnWizardFullscreen = nullptr;

    // Explorer Tab UI
    struct ExplorerCardWidgets {
        PatternType type;
        QLabel *titleLabel;
        QLabel *descLabel;
        QPushButton *actionBtn;
    };
    QVector<ExplorerCardWidgets> m_explorerCards;

    // Hardware Tab UI
    QGroupBox *m_groupScreenInfo = nullptr;
    QLabel *m_lblScreenModelHeader = nullptr;
    QLabel *m_lblScreenModel = nullptr;
    QLabel *m_lblScreenResHeader = nullptr;
    QLabel *m_lblScreenRes = nullptr;
    QLabel *m_lblScreenRateHeader = nullptr;
    QLabel *m_lblScreenRate = nullptr;
    QLabel *m_lblScreenDpiHeader = nullptr;
    QLabel *m_lblScreenDpi = nullptr;

    QGroupBox *m_groupHwCtrl = nullptr;
    QLabel *m_lblDdcStatus = nullptr;
    QLabel *m_lblBrightnessHeader = nullptr;
    QSlider *m_sliderBrightness = nullptr;
    QLabel *m_lblBrightnessVal = nullptr;
    QLabel *m_lblContrastHeader = nullptr;
    QSlider *m_sliderContrast = nullptr;
    QLabel *m_lblContrastVal = nullptr;
    QPushButton *m_btnResetSoftware = nullptr;

    // Fullscreen Pattern Window
    QWidget *m_fullscreenWindow = nullptr;
    PatternWidget *m_fullscreenPatternWidget = nullptr;
    bool m_isFullscreenWizard = false;
};
