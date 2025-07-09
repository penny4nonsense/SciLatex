#include "BracketChooserDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QRadioButton>
#include <QDialogButtonBox>

#include "BracketChooserDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QRadioButton>
#include <QDialogButtonBox>

BracketChooserDialog::BracketChooserDialog(QWidget *parent)
    : QDialog(parent), settings("YourCompany", "YourApp")
{
    setWindowTitle("Choose Brackets");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Create horizontal layout for side-by-side sections
    QHBoxLayout *sideBySideLayout = new QHBoxLayout;

    // Left Bracket Section
    QVBoxLayout *leftLayout = new QVBoxLayout;
    QLabel *leftLabel = new QLabel("Left Bracket:");
    leftBracketGroup = new QButtonGroup(this);

    QStringList leftBrackets = {"{", "[", "(", "|", "<", "||", "⟨", ""};
    for (const QString &bracket : leftBrackets) {
        QRadioButton *button = new QRadioButton(bracket.isEmpty() ? "None" : bracket);
        leftBracketGroup->addButton(button);
        leftLayout->addWidget(button);
    }
    sideBySideLayout->addLayout(leftLayout);

    // Right Bracket Section
    QVBoxLayout *rightLayout = new QVBoxLayout;
    QLabel *rightLabel = new QLabel("Right Bracket:");
    rightBracketGroup = new QButtonGroup(this);

    QStringList rightBrackets = {"}", "]", ")", "|", ">", "||", "⟩", ""};
    for (const QString &bracket : rightBrackets) {
        QRadioButton *button = new QRadioButton(bracket.isEmpty() ? "None" : bracket);
        rightBracketGroup->addButton(button);
        rightLayout->addWidget(button);
    }
    sideBySideLayout->addLayout(rightLayout);

    // Add side-by-side layout to main layout
    mainLayout->addLayout(sideBySideLayout);

    // Dialog Buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // Load last selections
    loadLastSelections();
    connect(this, &QDialog::accepted, this, &BracketChooserDialog::saveLastSelections);
}


void BracketChooserDialog::loadLastSelections()
{
    lastLeftBracket = settings.value("LastLeftBracket", "").toString();
    lastRightBracket = settings.value("LastRightBracket", "").toString();

    for (QAbstractButton *button : leftBracketGroup->buttons()) {
        if (button->text() == lastLeftBracket || (lastLeftBracket.isEmpty() && button->text() == "None")) {
            button->setChecked(true);
            break;
        }
    }

    for (QAbstractButton *button : rightBracketGroup->buttons()) {
        if (button->text() == lastRightBracket || (lastRightBracket.isEmpty() && button->text() == "None")) {
            button->setChecked(true);
            break;
        }
    }
}

void BracketChooserDialog::saveLastSelections()
{
    QAbstractButton *leftButton = leftBracketGroup->checkedButton();
    QAbstractButton *rightButton = rightBracketGroup->checkedButton();

    lastLeftBracket = leftButton ? leftButton->text() : "";
    lastRightBracket = rightButton ? rightButton->text() : "";

    settings.setValue("LastLeftBracket", lastLeftBracket);
    settings.setValue("LastRightBracket", lastRightBracket);
}

QString BracketChooserDialog::getLeftBracket() const
{
    return leftBracketGroup->checkedButton()->text();
}

QString BracketChooserDialog::getRightBracket() const
{
    return rightBracketGroup->checkedButton()->text();
}

QString BracketChooserDialog::toLaTeX(const QString &left, const QString &right) const
{
    // Define mapping for special characters
    QMap<QString, QString> latexMap = {
        {"{", "\\{"},
        {"}", "\\}"},
        {"|", "|"},
        {"||", "\\|"},
        {"⟨", "\\langle"},
        {"⟩", "\\rangle"},
        {"", "."} // No visible line maps to '.'
    };

    QString leftBracket = latexMap.value(left, left); // Default to original if not in map
    QString rightBracket = latexMap.value(right, right);

    // Return LaTeX command
    return QString("\\left%1$1\\right%2").arg(leftBracket, rightBracket);
}
