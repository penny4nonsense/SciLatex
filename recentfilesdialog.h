#ifndef RECENTFILESDIALOG_H
#define RECENTFILESDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>

class RecentFilesDialog : public QDialog {
    Q_OBJECT

public:
    explicit RecentFilesDialog(QWidget *parent = nullptr);
    void setRecentFiles(const QStringList &files);

signals:
    void fileSelected(const QString &filePath);
    void clearRequested();

private:
    QListWidget *recentFilesListWidget;
    QPushButton *openButton;
    QPushButton *clearButton;
    QPushButton *closeButton;

private slots:
    void onFileDoubleClicked(QListWidgetItem *item);
    void onOpenButtonClicked();
};

#endif // RECENTFILESDIALOG_H
