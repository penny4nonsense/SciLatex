#include "decoratordialog.h"

DecoratorDialog::DecoratorDialog(QWidget *parent) : QDialog(parent), selectedDecorator("") {
    setWindowTitle("Insert Decoration");

    // List of decorators
    decoratorList = new QListWidget(this);
    decoratorList->addItem("Overline");
    decoratorList->addItem("Underline");
    decoratorList->addItem("Hat");
    decoratorList->addItem("Tilde");
    decoratorList->addItem("Dot");
    decoratorList->addItem("Double Dot");
    decoratorList->addItem("Right Arrow (above)");
    decoratorList->addItem("Left Arrow (above)");
    decoratorList->addItem("Double Arrow (above)");
    decoratorList->addItem("Right Arrow (below)");
    decoratorList->addItem("Left Arrow (below)");
    decoratorList->addItem("Double Arrow (below)");

    // Buttons
    okButton = new QPushButton("Insert", this);
    cancelButton = new QPushButton("Cancel", this);

    // Layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(decoratorList);
    layout->addWidget(okButton);
    layout->addWidget(cancelButton);
    setLayout(layout);

    // Connect buttons
    connect(okButton, &QPushButton::clicked, [this]() {
        if (decoratorList->currentItem()) {
            selectedDecorator = decoratorList->currentItem()->text();
            accept();
        }
    });

    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

QString DecoratorDialog::getSelectedDecorator() const {
    // Map the descriptive name back to its LaTeX command
    if (selectedDecorator.contains("Overline")) return "\\overline{$1}";
    if (selectedDecorator.contains("Underline")) return "\\underline{$1}";
    if (selectedDecorator.contains("Hat")) return "\\hat{$1}";
    if (selectedDecorator.contains("Tilde")) return "\\tilde{$1}";
    if (selectedDecorator.contains("Dot")) return "\\dot{$1}";
    if (selectedDecorator.contains("Double Dot")) return "\\ddot{$1}";
    if (selectedDecorator.contains("Right Arrow (above)")) return "\\overrightarrow{$1}";
    if (selectedDecorator.contains("Left Arrow (above)")) return "\\overleftarrow{$1}";
    if (selectedDecorator.contains("Double Arrow (above)")) return "\\overleftrightarrow{$1}";
    if (selectedDecorator.contains("Right Arrow (below)")) return "\\underrightarrow{$1}";
    if (selectedDecorator.contains("Left Arrow (below)")) return "\\underleftarrow{$1}";
    if (selectedDecorator.contains("Double Arrow (below)")) return "\\underleftrightarrow{$1}";
    return "";
}
