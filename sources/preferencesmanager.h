#ifndef PREFERENCESMANAGER_H
#define PREFERENCESMANAGER_H

#include <QString>
#include <QJsonObject>
#include <QDir>
#include <QStandardPaths>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>

class PreferencesManager {
public:
    static QString getConfigPath();
    static QJsonObject loadPreferences();
    static void savePreferences(const QJsonObject &preferences);
    static QJsonObject getDefaultPreferences();
    static QString getApiKey();
    void setApiKey(const QString &apiKey);
    static QString getMatlabPath();
};

#endif // PREFERENCESMANAGER_H
