#include "previewwindow.h"
#include <QFile>
#include <QDebug>

PreviewWindow::PreviewWindow(const QString &pdfFilePath, QWidget *parent)
    : QMainWindow(parent),
    pdfDocument(new QPdfDocument(this)),
    pdfView(new QPdfView(this)) {

    setWindowTitle("PDF Preview");
    resize(800, 600);

    // Load the PDF file
    if (QFile::exists(pdfFilePath)) {
        if (pdfDocument->load(pdfFilePath) == QPdfDocument::Error::None) {
            pdfView->setDocument(pdfDocument);
            pdfView->setPageMode(QPdfView::PageMode::SinglePage);
        } else {
            qDebug() << "[ERROR] Failed to load PDF file:" << pdfFilePath;
        }
    } else {
        qDebug() << "[ERROR] PDF file does not exist:" << pdfFilePath;
    }

    // Set layout
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(pdfView);

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);
}
