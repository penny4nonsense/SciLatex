#include "preferencesmanager.h"
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <stdexcept>

QString PreferencesManager::getConfigPath() {
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/config.json";
    return configPath;
}

QJsonObject PreferencesManager::loadPreferences() {
    QString configPath = getConfigPath();
    QFile file(configPath);

    if (!file.exists()) {
        return getDefaultPreferences();
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error("Unable to open config file for reading.");
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (!doc.isObject()) {
        throw std::runtime_error("Invalid config file format.");
    }

    return doc.object();
}

void PreferencesManager::savePreferences(const QJsonObject &preferences) {
    QString configPath = getConfigPath();
    QFileInfo fileInfo(configPath);
    QDir dir = fileInfo.dir();

    // Ensure the directory exists
    if (!dir.exists() && !dir.mkpath(dir.path())) {
        throw std::runtime_error("Unable to create config directory.");
    }

    QFile file(configPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw std::runtime_error("Unable to open config file for writing.");
    }

    QJsonDocument doc(preferences);
    if (file.write(doc.toJson()) == -1) {
        throw std::runtime_error("Failed to write to config file.");
    }
    file.close();
}

QJsonObject PreferencesManager::getDefaultPreferences() {
    QJsonObject defaultPrefs;
    defaultPrefs["defaultFontSize"] = 12;
    defaultPrefs["latexInterpreterPath"] = "pdflatex";
    defaultPrefs["matlabPath"] = "matlab";
    defaultPrefs["recentFiles"] = QJsonArray();
    defaultPrefs["apiKey"] = ""; // Add API key default
    return defaultPrefs;
}

QString PreferencesManager::getApiKey() {
    QJsonObject preferences = loadPreferences();
    if (preferences.contains("apiKey")) {
        return preferences["apiKey"].toString();
    }
    return ""; // Return an empty string if the API key is not set
}

void PreferencesManager::setApiKey(const QString &apiKey) {
    QJsonObject preferences = loadPreferences();
    preferences["apiKey"] = apiKey;
    savePreferences(preferences);
}

QString PreferencesManager::getMatlabPath() {
    QJsonObject preferences = loadPreferences();
    if (preferences.contains("matlabPath")) {
        return preferences["matlabPath"].toString();
    } else {
        return ""; // Return an empty string if not found
    }
}
