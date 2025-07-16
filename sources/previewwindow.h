#ifndef PREVIEWWINDOW_H
#define PREVIEWWINDOW_H

#include <QMainWindow>
#include <QtPdfWidgets/QPdfView>
#include <QtPdf/QPdfDocument>
#include <QVBoxLayout>

class PreviewWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit PreviewWindow(const QString &pdfFilePath, QWidget *parent = nullptr);

private:
    QPdfDocument *pdfDocument;
    QPdfView *pdfView;
};

#endif // PREVIEWWINDOW_H
