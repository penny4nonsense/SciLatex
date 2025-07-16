#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QLineEdit>
#include <QFormLayout>
#include <QPushButton>
#include <QJsonObject>

class PreferencesDialog : public QDialog {
    Q_OBJECT // Required for Qt's meta-object system

public:
    explicit PreferencesDialog(QJsonObject &preferences, QWidget *parent = nullptr);
    QJsonObject getUpdatedPreferences() const;

private:
    QSpinBox *fontSizeSpinBox;
    QLineEdit *latexPathEdit;
    QLineEdit *matlabPathEdit;
    QLineEdit *apiKeyEdit;
    QJsonObject &prefs;
};

#endif // PREFERENCESDIALOG_H
