#include "preferencesdialog.h"
#include <QVBoxLayout>
#include <QLabel>

PreferencesDialog::PreferencesDialog(QJsonObject &preferences, QWidget *parent)
    : QDialog(parent), prefs(preferences) {
    setWindowTitle(tr("Preferences"));

    // Create form layout
    QFormLayout *formLayout = new QFormLayout;

    // Font size spin box
    fontSizeSpinBox = new QSpinBox(this);
    fontSizeSpinBox->setRange(6, 72); // Allow font sizes between 6 and 72
    fontSizeSpinBox->setValue(prefs.value("defaultFontSize").toInt(12)); // Default to 12 if not set
    formLayout->addRow(tr("Default Font Size:"), fontSizeSpinBox);

    // LaTeX interpreter path
    latexPathEdit = new QLineEdit(this);
    latexPathEdit->setText(prefs.value("latexInterpreterPath").toString());
    formLayout->addRow(tr("LaTeX Interpreter Path:"), latexPathEdit);

    // MATLAB path
    matlabPathEdit = new QLineEdit(this);
    matlabPathEdit->setText(prefs.value("matlabPath").toString());
    formLayout->addRow(tr("MATLAB Path:"), matlabPathEdit);

    apiKeyEdit = new QLineEdit(this);
    apiKeyEdit->setText(prefs.value("apiKey").toString());
    formLayout->addRow(tr("Open AI API Key:"), apiKeyEdit);

    // Buttons
    QPushButton *okButton = new QPushButton(tr("OK"), this);
    QPushButton *cancelButton = new QPushButton(tr("Cancel"), this);

    connect(okButton, &QPushButton::clicked, this, &PreferencesDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &PreferencesDialog::reject);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
}

QJsonObject PreferencesDialog::getUpdatedPreferences() const {
    QJsonObject updatedPrefs = prefs;

    updatedPrefs["defaultFontSize"] = fontSizeSpinBox->value();
    updatedPrefs["latexInterpreterPath"] = latexPathEdit->text();
    updatedPrefs["matlabPath"] = matlabPathEdit->text();
    updatedPrefs["apiKey"] = apiKeyEdit->text();

    return updatedPrefs;
}
