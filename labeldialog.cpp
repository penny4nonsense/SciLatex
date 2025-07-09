#include "labeldialog.h"

LabelDialog::LabelDialog(QWidget *parent) : QDialog(parent), selectedLabel("") {
    setWindowTitle("Insert Label");

    // Instruction Label
    QLabel *instruction = new QLabel("Choose label placement:", this);

    // Buttons for Above and Below options
    aboveButton = new QPushButton("Label Above", this);
    belowButton = new QPushButton("Label Below", this);

    // Layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(instruction);
    layout->addWidget(aboveButton);
    layout->addWidget(belowButton);
    setLayout(layout);

    // Connect buttons
    connect(aboveButton, &QPushButton::clicked, [this]() {
        selectedLabel = "overset";
        accept(); // Close the dialog and return success
    });

    connect(belowButton, &QPushButton::clicked, [this]() {
        selectedLabel = "underset";
        accept();
    });
}

QString LabelDialog::getSelectedLabelType() const {
    return selectedLabel;
}
