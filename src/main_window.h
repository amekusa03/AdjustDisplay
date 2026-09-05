#pragma once

#include <QMainWindow>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QTabWidget>
#include <QListWidget>
#include <QGroupBox>
#include <QStackedWidget>
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

    // UI Widgets
    QComboBox *m_screenCombo = nullptr;
    QPushButton *m_btnIdentify = nullptr;
    QPushButton *m_btnIdentifyAll = nullptr;
    QPushButton *m_btnRefreshScreens = nullptr;

    QTabWidget *m_tabWidget = nullptr;

    // Wizard Tab UI
    QListWidget *m_stepList = nullptr;
    QLabel *m_wizardTitleLabel = nullptr;
    QLabel *m_wizardDescLabel = nullptr;
    QLabel *m_wizardOsdTipLabel = nullptr;
    PatternWidget *m_previewPatternWidget = nullptr;
    QPushButton *m_btnWizardPrev = nullptr;
    QPushButton *m_btnWizardNext = nullptr;
    QPushButton *m_btnWizardFullscreen = nullptr;

    // Hardware Tab UI
    QLabel *m_lblScreenModel = nullptr;
    QLabel *m_lblScreenRes = nullptr;
    QLabel *m_lblScreenDpi = nullptr;
    QLabel *m_lblScreenRate = nullptr;
    QLabel *m_lblDdcStatus = nullptr;
    QSlider *m_sliderBrightness = nullptr;
    QLabel *m_lblBrightnessVal = nullptr;
    QSlider *m_sliderContrast = nullptr;
    QLabel *m_lblContrastVal = nullptr;
    QPushButton *m_btnResetSoftware = nullptr;

    // Fullscreen Pattern Window
    QWidget *m_fullscreenWindow = nullptr;
    PatternWidget *m_fullscreenPatternWidget = nullptr;
    bool m_isFullscreenWizard = false;
};
