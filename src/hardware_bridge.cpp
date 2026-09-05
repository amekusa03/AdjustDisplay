#include "hardware_bridge.h"
#include <QStandardPaths>
#include <QDebug>

HardwareBridge::HardwareBridge(QObject *parent) : QObject(parent) {
    m_ddcQueueTimer.setSingleShot(true);
    m_ddcQueueTimer.setInterval(200); // 200ms debounce: DDC/CI は I2C 通信のため連続送信不可
    connect(&m_ddcQueueTimer, &QTimer::timeout, this, &HardwareBridge::processQueuedDdcCommands);
}

HardwareBridge::~HardwareBridge() = default;

void HardwareBridge::probeCapabilities(const QString &outputName) {
    // [修正 #2] 再入時: 実行中の probe プロセスがあれば中断してから再開する
    if (m_probeProcess && m_probeProcess->state() != QProcess::NotRunning) {
        m_probeProcess->disconnect(); // finished シグナルを切り離して古い結果を無視
        m_probeProcess->kill();
        m_probeProcess->waitForFinished(100);
    }

    m_currentOutput = outputName;
    m_caps = HardwareCapabilities();

    // Check if ddcutil exists
    QString ddcPath = QStandardPaths::findExecutable("ddcutil");
    m_caps.hasDdcUtil = !ddcPath.isEmpty();

    // Check if xrandr exists
    QString xrandrPath = QStandardPaths::findExecutable("xrandr");
    m_caps.hasXrandr = !xrandrPath.isEmpty();

    if (m_caps.hasDdcUtil) {
        auto *proc = new QProcess(this);
        m_probeProcess = proc; // 追跡用に保存

        // [修正 #1] connect() を start() より前に行い、超高速終了時のシグナル取りこぼしを防ぐ
        connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, [this, proc](int exitCode, QProcess::ExitStatus) {
            proc->deleteLater();

            if (exitCode != 0 || !proc->readAllStandardOutput().contains("Display")) {
                m_caps.ddcResponsive = false;
                m_caps.errorMessage = "DDC/CIは利用できません (i2c-dev権限または非対応ディスプレイ)";
                if (exitCode != 0) {
                    qDebug() << "[HardwareBridge] ddcutil detect stderr:" << proc->readAllStandardError();
                }
                emit capabilitiesUpdated(m_caps);
                return;
            }

            m_caps.ddcResponsive = true;

            // Query current brightness — connect() してから start()
            auto *valProc = new QProcess(this);
            connect(valProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                    this, [this, valProc](int code, QProcess::ExitStatus) {
                if (code == 0) {
                    QString v = valProc->readAllStandardOutput();
                    // ddcutil --terse 出力例: "VCP 10 C 60 100"
                    // フィールド: [0]=VCP [1]=featureCode [2]=type [3]=current [4]=max
                    const auto parts = v.trimmed().split(' ');
                    if (parts.size() >= 5) {
                        bool ok1 = false, ok2 = false;
                        int cur = parts[3].toInt(&ok1);
                        int max = parts[4].toInt(&ok2);
                        if (ok1 && ok2 && max > 0) {
                            m_caps.currentBrightness = cur;
                            m_caps.maxBrightness = max;
                        }
                    }
                } else {
                    qDebug() << "[HardwareBridge] getvcp VCP 10 failed, code=" << code
                             << valProc->readAllStandardError();
                }
                valProc->deleteLater();
                emit capabilitiesUpdated(m_caps);
            });
            valProc->start("ddcutil", QStringList() << "getvcp" << "10" << "--terse");
        });

        proc->start("ddcutil", QStringList() << "detect" << "--terse");
    } else {
        emit capabilitiesUpdated(m_caps);
    }
}

void HardwareBridge::setBrightness(int value, const QString &outputName) {
    // NOTE: m_caps.currentBrightness は UI 即時反映のため先に更新する。
    //       実際のモニタへの反映はデバウンス後の processQueuedDdcCommands() で行われる。
    m_caps.currentBrightness = value;
    emit brightnessChanged(value);

    if (m_caps.hasDdcUtil && m_caps.ddcResponsive) {
        m_pendingBrightness = value;
        m_ddcQueueTimer.start();
    } else if (m_caps.hasXrandr && !outputName.isEmpty()) {
        // XRandR ソフトウェア補正: スライダー 0〜100 を xrandr の 0.0〜1.0 にマッピング
        // [修正 #3] 上限を 1.0 に修正（スライダーの範囲で 1.5 に達することはなく意図不明だったため）
        double norm = qBound(0.0, value / 100.0, 1.0);
        QProcess::startDetached("xrandr", QStringList()
                                    << "--output" << outputName
                                    << "--brightness" << QString::number(norm, 'f', 2));
    }
}

void HardwareBridge::setContrast(int value) {
    m_caps.currentContrast = value;
    emit contrastChanged(value);

    if (m_caps.hasDdcUtil && m_caps.ddcResponsive) {
        m_pendingContrast = value;
        m_ddcQueueTimer.start();
    }
}

void HardwareBridge::setSoftwareGamma(double gamma, const QString &outputName) {
    if (m_caps.hasXrandr && !outputName.isEmpty() && gamma > 0.1) {
        double gVal = 1.0 / gamma;
        QString gStr = QString("%1:%1:%1").arg(QString::number(gVal, 'f', 2));
        QProcess::startDetached("xrandr", QStringList()
                                    << "--output" << outputName
                                    << "--gamma" << gStr);
    }
}

void HardwareBridge::resetSoftwareSettings(const QString &outputName) {
    if (m_caps.hasXrandr && !outputName.isEmpty()) {
        QProcess::startDetached("xrandr", QStringList()
                                    << "--output" << outputName
                                    << "--brightness" << "1.0"
                                    << "--gamma" << "1.0:1.0:1.0");
    }
}

// [修正 #4] DDC/CI コマンド失敗時にエラーをシグナルで通知する
void HardwareBridge::onDdcProcessFinished(int exitCode, QProcess::ExitStatus) {
    m_ddcBusy = false;
    if (exitCode != 0) {
        emit statusMessage(
            QString("DDC/CI コマンドが失敗しました (終了コード: %1)").arg(exitCode),
            /*isError=*/true
        );
    }
    processQueuedDdcCommands();
}

void HardwareBridge::processQueuedDdcCommands() {
    if (m_ddcBusy) return;

    if (m_pendingBrightness >= 0) {
        int b = m_pendingBrightness;
        m_pendingBrightness = -1;
        m_ddcBusy = true;
        auto *proc = new QProcess(this);
        connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, [this, proc](int code, QProcess::ExitStatus st) {
            proc->deleteLater();
            onDdcProcessFinished(code, st);
        });
        proc->start("ddcutil", QStringList() << "setvcp" << "10" << QString::number(b));
        return;
    }

    if (m_pendingContrast >= 0) {
        int c = m_pendingContrast;
        m_pendingContrast = -1;
        m_ddcBusy = true;
        auto *proc = new QProcess(this);
        connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, [this, proc](int code, QProcess::ExitStatus st) {
            proc->deleteLater();
            onDdcProcessFinished(code, st);
        });
        proc->start("ddcutil", QStringList() << "setvcp" << "12" << QString::number(c));
        return;
    }
}
