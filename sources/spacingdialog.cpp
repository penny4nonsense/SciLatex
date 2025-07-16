#include "spacingdialog.h"

SpacingDialog::SpacingDialog(QWidget *parent) : QDialog(parent), selectedSpacing("") {
    setWindowTitle("Insert Spacing");

    // Spacing options with user-friendly names
    spacingList = new QListWidget(this);
    spacingList->addItem("Space");
    spacingList->addItem("Thin Space");
    spacingList->addItem("Negative Thin Space");
    spacingList->addItem("En Dash");
    spacingList->addItem("Em Dash");
    spacingList->addItem("Quad");
    spacingList->addItem("No Indent");
    spacingList->addItem("Horizontal Fill");

    spacingList->addItem("Small Skip");
    spacingList->addItem("Medium Skip");
    spacingList->addItem("Big Skip");
    spacingList->addItem("Vertical Fill");
    spacingList->addItem("Line Break");
    spacingList->addItem("Page Break");

    // Buttons
    okButton = new QPushButton("Insert", this);
    cancelButton = new QPushButton("Cancel", this);

    // Layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(spacingList);
    layout->addWidget(okButton);
    layout->addWidget(cancelButton);
    setLayout(layout);

    // Connect buttons
    connect(okButton, &QPushButton::clicked, [this]() {
        if (spacingList->currentItem()) {
            selectedSpacing = spacingList->currentItem()->text();
            accept();
        }
    });

    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

QString SpacingDialog::getSelectedSpacing() const {
    // Map user-friendly names back to LaTeX commands
    if (selectedSpacing.contains("Space")) return "\\ ";
    if (selectedSpacing.contains("Thin Space")) return "\\;";
    if (selectedSpacing.contains("Negative Thin Space")) return "\\!";
    if (selectedSpacing.contains("En Dash")) return "--";
    if (selectedSpacing.contains("Em Dash")) return "---";
    if (selectedSpacing.contains("Quad")) return "\\quad";
    if (selectedSpacing.contains("No Indent")) return "\\noindent";
    if (selectedSpacing.contains("Horizontal Fill")) return "\\hfill";
    if (selectedSpacing.contains("Small Skip")) return "\\smallskip";
    if (selectedSpacing.contains("Medium Skip")) return "\\medskip";
    if (selectedSpacing.contains("Big Skip")) return "\\bigskip";
    if (selectedSpacing.contains("Vertical Fill")) return "\\vfill";
    if (selectedSpacing.contains("Line Break")) return "\\\\";
    if (selectedSpacing.contains("Page Break")) return "\\newpage";
    return "";
}
