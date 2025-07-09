#include "fontdialog.h"
#include "fontdialog.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QHBoxLayout>

FontDialog::FontDialog(QWidget *parent)
    : QDialog(parent), fontList(new QListWidget(this)) {
    setWindowTitle("Select Font Style");

    QVBoxLayout *layout = new QVBoxLayout(this);

    fontList->addItems({
        "Bold (\\mathbf)", "Roman (\\mathrm)", "Caligraphic (\\mathcal)",
        "Blackboard Bold (\\mathbb)", "Sans Serif (\\mathsf)",
        "Fraktur (\\mathfrak)", "Italic (\\mathit)", "Bold Italic (\\bm)"
    });

    layout->addWidget(fontList);

    QPushButton *okButton = new QPushButton("OK", this);
    QPushButton *cancelButton = new QPushButton("Cancel", this);

    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    layout->addLayout(buttonLayout);
}

QString FontDialog::getSelectedFontCommand() const {
    if (fontList->currentItem()) {
        QString selectedFont = fontList->currentItem()->text();
        if (selectedFont.startsWith("Bold")) return "\\mathbf";
        if (selectedFont.startsWith("Roman")) return "\\mathrm";
        if (selectedFont.startsWith("Caligraphic")) return "\\mathcal";
        if (selectedFont.startsWith("Blackboard Bold")) return "\\mathbb";
        if (selectedFont.startsWith("Sans Serif")) return "\\mathsf";
        if (selectedFont.startsWith("Fraktur")) return "\\mathfrak";
        if (selectedFont.startsWith("Italic")) return "\\mathit";
        if (selectedFont.startsWith("Bold Italic")) return "\\bm";
    }
    return QString();
}
