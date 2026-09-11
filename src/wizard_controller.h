#pragma once

#include <QObject>
#include <QVector>
#include "pattern_types.h"

class WizardController : public QObject {
    Q_OBJECT
public:
    explicit WizardController(QObject *parent = nullptr);
    ~WizardController() override;

    int currentStepIndex() const { return m_currentStepIndex; }
    int totalSteps() const { return m_steps.size(); }
    const WizardStepInfo& currentStepInfo() const;
    const QVector<WizardStepInfo>& allSteps() const { return m_steps; }

    void nextStep();
    void prevStep();
    void goToStep(int index);
    void reset();
    void reloadSteps();

signals:
    void stepChanged(int stepIndex, const WizardStepInfo &info);
    void wizardFinished();

private:
    void setupSteps();

    QVector<WizardStepInfo> m_steps;
    int m_currentStepIndex = 0;
};
