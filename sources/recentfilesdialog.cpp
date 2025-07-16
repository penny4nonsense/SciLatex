#include "recentfilesdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QMessageBox>

RecentFilesDialog::RecentFilesDialog(QWidget *parent)
    : QDialog(parent),
    recentFilesListWidget(new QListWidget(this)),
    openButton(new QPushButton(tr("Open"), this)),
    clearButton(new QPushButton(tr("Clear List"), this)),
    closeButton(new QPushButton(tr("Close"), this)) {

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // List widget for recent files
    mainLayout->addWidget(recentFilesListWidget);

    // Button layout
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(openButton);
    buttonLayout->addWidget(clearButton);
    buttonLayout->addWidget(closeButton);
    mainLayout->addLayout(buttonLayout);

    // Connections
    connect(recentFilesListWidget, &QListWidget::itemDoubleClicked, this, &RecentFilesDialog::onFileDoubleClicked);
    connect(openButton, &QPushButton::clicked, this, &RecentFilesDialog::onOpenButtonClicked);
    connect(clearButton, &QPushButton::clicked, this, &RecentFilesDialog::clearRequested);
    connect(closeButton, &QPushButton::clicked, this, &RecentFilesDialog::close);
}

void RecentFilesDialog::setRecentFiles(const QStringList &files) {
    recentFilesListWidget->clear();
    recentFilesListWidget->addItems(files);
}

void RecentFilesDialog::onFileDoubleClicked(QListWidgetItem *item) {
    emit fileSelected(item->text());
    accept(); // Close dialog
}

void RecentFilesDialog::onOpenButtonClicked() {
    QListWidgetItem *selectedItem = recentFilesListWidget->currentItem();
    if (selectedItem) {
        emit fileSelected(selectedItem->text());
        accept(); // Close dialog
    } else {
        QMessageBox::warning(this, tr("No Selection"), tr("Please select a file to open."));
    }
}
