#include "OperatorDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>

OperatorDialog::OperatorDialog(QWidget *parent)
    : QDialog(parent) {
    // Operator Selection
    operatorBox = new QComboBox(this);
    operatorBox->addItem("Summation (\\sum)", "\\sum");
    operatorBox->addItem("Product (\\prod)", "\\prod");
    operatorBox->addItem("Union (\\bigcup)", "\\bigcup");
    operatorBox->addItem("Intersection (\\bigcap)", "\\bigcap");
    operatorBox->addItem("Coproduct (\\coprod)", "\\coprod");
    operatorBox->addItem("Integral (\\int)", "\\int");
    operatorBox->addItem("Double Integral (\\iint)", "\\iint");
    operatorBox->addItem("Triple Integral (\\iiint)", "\\iiint");
    operatorBox->addItem("Contour Integral (\\oint)", "\\oint");
    operatorBox->addItem("Surface Integral (\\oiint)", "\\oiint");
    operatorBox->addItem("Volume Integral (\\oiiint)", "\\oiiint");
    operatorBox->addItem("Disjoint Union (\\bigsqcup)", "\\bigsqcup");
    operatorBox->addItem("Logical OR (\\bigvee)", "\\bigvee");
    operatorBox->addItem("Logical AND (\\bigwedge)", "\\bigwedge");
    operatorBox->addItem("Big Circle Dot (\\bigodot)", "\\bigodot");
    operatorBox->addItem("Tensor Product (\\bigotimes)", "\\bigotimes");
    operatorBox->addItem("Direct Sum (\\bigoplus)", "\\bigoplus");
    operatorBox->addItem("Limit (\\lim)", "\\lim");
    operatorBox->addItem("Limit Superior (\\limsup)", "\\limsup");
    operatorBox->addItem("Limit Inferior (\\liminf)", "\\liminf");
    operatorBox->addItem("Supremum (\\sup)", "\\sup");
    operatorBox->addItem("Infimum (\\inf)", "\\inf");
    operatorBox->addItem("Maximum (\\max)", "\\max");
    operatorBox->addItem("Minimum (\\min)", "\\min");
    operatorBox->addItem("Determinant (\\det)", "\\det");
    operatorBox->addItem("Greatest Common Divisor (\\gcd)", "\\gcd");
    operatorBox->addItem("Degree (\\deg)", "\\deg");
    operatorBox->addItem("Kernel (\\ker)", "\\ker");
    operatorBox->addItem("Dimension (\\dim)", "\\dim");
    operatorBox->addItem("Homomorphism (\\hom)", "\\hom");
    operatorBox->addItem("Natural Logarithm (\\ln)", "\\ln");
    operatorBox->addItem("Logarithm (\\log)", "\\log");
    operatorBox->addItem("Exponential (\\exp)", "\\exp");
    // Add other operators here...

    // Limit Placement Options
    QGroupBox *limitGroup = new QGroupBox("Limit Placement", this);
    QVBoxLayout *limitLayout = new QVBoxLayout();
    autoLimit = new QRadioButton("Auto (Default)", this);
    aboveBelowLimit = new QRadioButton("Above/Below (\\limits)", this);
    rightLimit = new QRadioButton("Right (\\nolimits)", this);
    autoLimit->setChecked(true);
    limitLayout->addWidget(autoLimit);
    limitLayout->addWidget(aboveBelowLimit);
    limitLayout->addWidget(rightLimit);
    limitGroup->setLayout(limitLayout);

    // Size Options
    QGroupBox *sizeGroup = new QGroupBox("Size", this);
    QVBoxLayout *sizeLayout = new QVBoxLayout();
    autoSize = new QRadioButton("Auto", this);
    bigSize = new QRadioButton("Big (\\displaystyle)", this);
    smallSize = new QRadioButton("Small (\\textstyle)", this);
    autoSize->setChecked(true);
    sizeLayout->addWidget(autoSize);
    sizeLayout->addWidget(bigSize);
    sizeLayout->addWidget(smallSize);
    sizeGroup->setLayout(sizeLayout);

    // Preview Area
    previewLabel = new QLabel("Preview: \\sum_{i=1}^n i", this);

    // Buttons
    QPushButton *okButton = new QPushButton("OK", this);
    QPushButton *cancelButton = new QPushButton("Cancel", this);
    connect(okButton, &QPushButton::clicked, this, &OperatorDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &OperatorDialog::reject);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addWidget(new QLabel("Select an operator:", this));
    mainLayout->addWidget(operatorBox);
    mainLayout->addWidget(limitGroup);
    mainLayout->addWidget(sizeGroup);
    mainLayout->addWidget(previewLabel);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
    setWindowTitle("Operator Selector");
    resize(400, 300);

    // Connect signals for dynamic preview updates
    connect(operatorBox, &QComboBox::currentTextChanged, this, &OperatorDialog::updatePreview);
    connect(autoLimit, &QRadioButton::toggled, this, &OperatorDialog::updatePreview);
    connect(aboveBelowLimit, &QRadioButton::toggled, this, &OperatorDialog::updatePreview);
    connect(rightLimit, &QRadioButton::toggled, this, &OperatorDialog::updatePreview);
    connect(autoSize, &QRadioButton::toggled, this, &OperatorDialog::updatePreview);
    connect(bigSize, &QRadioButton::toggled, this, &OperatorDialog::updatePreview);
    connect(smallSize, &QRadioButton::toggled, this, &OperatorDialog::updatePreview);
}

void OperatorDialog::updatePreview() {
    QString operatorName = operatorBox->currentData().toString();
    QString limits;
    if (aboveBelowLimit->isChecked()) {
        limits = "\\limits";
    } else if (rightLimit->isChecked()) {
        limits = "\\nolimits";
    }
    QString size;
    if (bigSize->isChecked()) {
        size = "\\displaystyle ";
    } else if (smallSize->isChecked()) {
        size = "\\textstyle ";
    }
    previewLabel->setText("Preview: " + size + operatorName + limits + "_{i=1}^n i");
}


QString OperatorDialog::getSelectedOperator() const {
    return operatorBox->currentData().toString();
}

QString OperatorDialog::getLimitPlacement() const {
    if (aboveBelowLimit->isChecked()) {
        return "\\limits";
    } else if (rightLimit->isChecked()) {
        return "\\nolimits";
    }
    return ""; // Auto (default placement)
}

QString OperatorDialog::getSize() const {
    if (bigSize->isChecked()) {
        return "\\displaystyle ";
    } else if (smallSize->isChecked()) {
        return "\\textstyle ";
    }
    return ""; // Auto (default size)
}
