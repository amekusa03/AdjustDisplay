#pragma once

#include <QObject>
#include <QString>
#include <QProcess>
#include <QTimer>
#include <QPointer>

struct HardwareCapabilities {
    bool hasDdcUtil = false;
    bool ddcResponsive = false;
    bool hasXrandr = false;
    int currentBrightness = -1; // -1 if unknown
    int maxBrightness = 100;
    int currentContrast = -1;   // -1 if unknown
    int maxContrast = 100;
    QString errorMessage;
};

class HardwareBridge : public QObject {
    Q_OBJECT
public:
    explicit HardwareBridge(QObject *parent = nullptr);
    ~HardwareBridge() override;

    void probeCapabilities(const QString &outputName = QString());

    bool isDdcAvailable() const { return m_caps.hasDdcUtil && m_caps.ddcResponsive; }
    bool isXrandrAvailable() const { return m_caps.hasXrandr; }
    const HardwareCapabilities& capabilities() const { return m_caps; }

    void setBrightness(int value, const QString &outputName = QString());
    void setContrast(int value);
    void setSoftwareGamma(double gamma, const QString &outputName = QString());
    void resetSoftwareSettings(const QString &outputName = QString());

signals:
    void capabilitiesUpdated(const HardwareCapabilities &caps);
    void brightnessChanged(int value);
    void contrastChanged(int value);
    void statusMessage(const QString &msg, bool isError = false);

private slots:
    void onDdcProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void processQueuedDdcCommands();

private:
    HardwareCapabilities m_caps;
    QTimer m_ddcQueueTimer;
    int m_pendingBrightness = -1;
    int m_pendingContrast = -1;
    bool m_ddcBusy = false;
    QString m_currentOutput;

    // 実行中の capability probe プロセスを追跡（再入時のキャンセルに使用）
    QPointer<QProcess> m_probeProcess;
};
