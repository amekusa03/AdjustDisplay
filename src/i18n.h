#pragma once

#include <QObject>
#include <QString>
#include <QSettings>
#include <QLocale>

enum class Language {
    English,
    Japanese
};

class I18n : public QObject {
    Q_OBJECT
public:
    static I18n* instance();

    Language language() const { return m_language; }
    void setLanguage(Language lang);

    bool isJapanese() const { return m_language == Language::Japanese; }

    // Helper translation getter
    QString t(const char *key) const;

signals:
    void languageChanged(Language lang);

private:
    explicit I18n(QObject *parent = nullptr);
    ~I18n() override = default;

    void loadSettings();
    void saveSettings();

    Language m_language = Language::English;
};
