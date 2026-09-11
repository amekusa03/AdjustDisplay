#include "wizard_controller.h"
#include "i18n.h"

WizardController::WizardController(QObject *parent) : QObject(parent) {
    setupSteps();
    connect(I18n::instance(), &I18n::languageChanged, this, [this]() {
        reloadSteps();
    });
}

WizardController::~WizardController() = default;

void WizardController::reloadSteps() {
    setupSteps();
    emit stepChanged(m_currentStepIndex, currentStepInfo());
}

void WizardController::setupSteps() {
    m_steps.clear();
    auto *i18n = I18n::instance();

    // Step 1: Black Level
    m_steps.append({
        PatternType::BlackLevel,
        i18n->t("step1_title"),
        i18n->t("step1_subtitle"),
        i18n->t("step1_inst"),
        i18n->t("step1_osd")
    });

    // Step 2: White Level
    m_steps.append({
        PatternType::WhiteLevel,
        i18n->t("step2_title"),
        i18n->t("step2_subtitle"),
        i18n->t("step2_inst"),
        i18n->t("step2_osd")
    });

    // Step 3: Gamma 2.2
    m_steps.append({
        PatternType::Gamma22,
        i18n->t("step3_title"),
        i18n->t("step3_subtitle"),
        i18n->t("step3_inst"),
        i18n->t("step3_osd")
    });

    // Step 4: Grayscale Ramp
    m_steps.append({
        PatternType::GrayRamp,
        i18n->t("step4_title"),
        i18n->t("step4_subtitle"),
        i18n->t("step4_inst"),
        i18n->t("step4_osd")
    });

    // Step 5: Sharpness
    m_steps.append({
        PatternType::Sharpness,
        i18n->t("step5_title"),
        i18n->t("step5_subtitle"),
        i18n->t("step5_inst"),
        i18n->t("step5_osd")
    });

    // Step 6: Uniformity
    m_steps.append({
        PatternType::ColorUniformity,
        i18n->t("step6_title"),
        i18n->t("step6_subtitle"),
        i18n->t("step6_inst"),
        i18n->t("step6_osd")
    });

    // Step 7: Geometry & 1:1 Pixel Mapping
    m_steps.append({
        PatternType::GeometryFocus,
        i18n->t("step7_title"),
        i18n->t("step7_subtitle"),
        i18n->t("step7_inst"),
        i18n->t("step7_osd")
    });
}

const WizardStepInfo& WizardController::currentStepInfo() const {
    if (m_currentStepIndex >= 0 && m_currentStepIndex < m_steps.size()) {
        return m_steps[m_currentStepIndex];
    }
    static WizardStepInfo empty;
    return empty;
}

void WizardController::nextStep() {
    if (m_currentStepIndex < m_steps.size() - 1) {
        m_currentStepIndex++;
        emit stepChanged(m_currentStepIndex, currentStepInfo());
    } else {
        emit wizardFinished();
    }
}

void WizardController::prevStep() {
    if (m_currentStepIndex > 0) {
        m_currentStepIndex--;
        emit stepChanged(m_currentStepIndex, currentStepInfo());
    }
}

void WizardController::goToStep(int index) {
    if (index >= 0 && index < m_steps.size() && index != m_currentStepIndex) {
        m_currentStepIndex = index;
        emit stepChanged(m_currentStepIndex, currentStepInfo());
    }
}

void WizardController::reset() {
    m_currentStepIndex = 0;
    emit stepChanged(m_currentStepIndex, currentStepInfo());
}
