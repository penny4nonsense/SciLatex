#include "mathnamedialog.h"

MathNameDialog::MathNameDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Insert Math Name");

    // Math name dropdown
    mathNameComboBox = new QComboBox(this);
    QStringList mathNames = {
        "arccos",  "arcsin",  "arctan",  "arg",  "cos",  "cosh",  "deg",  "det", "diag",
        "dim",  "exp",  "gcd",  "hom",  "Im",  "inf",  "ker",  "lg",  "lim",  "liminf",
        "limsup",  "ln",  "log",  "max",  "min",  "mod",  "Pr",  "proj",  "rank",  "Re",
        "sec",  "sin",  "sinh",  "sup",  "tan",  "tanh",  "tr",  "varinjlim",  "varliminf",
        "varlimsup",  "varprojlim"
    };
    mathNameComboBox->addItems(mathNames);
    mathNameComboBox->setEditable(true); // Make searchable

    // Limit placement dropdown
    limitPlacementComboBox = new QComboBox(this);
    limitPlacementComboBox->addItems({"auto", "at-right", "above-below"});

    // Buttons
    okButton = new QPushButton("Insert", this);
    cancelButton = new QPushButton("Cancel", this);

    // Layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(mathNameComboBox);

    QGroupBox *limitGroupBox = new QGroupBox("Limit Placement", this);
    QVBoxLayout *limitLayout = new QVBoxLayout(limitGroupBox);
    limitLayout->addWidget(limitPlacementComboBox);

    layout->addWidget(limitGroupBox);
    layout->addWidget(okButton);
    layout->addWidget(cancelButton);

    setLayout(layout);

    // Connect buttons
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

QString MathNameDialog::getSelectedMathName() const {
    return mathNameComboBox->currentText();
}

QString MathNameDialog::getLimitPlacement() const {
    return limitPlacementComboBox->currentText();
}
