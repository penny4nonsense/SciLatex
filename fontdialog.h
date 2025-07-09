#ifndef FONTDIALOG_H
#define FONTDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

class FontDialog : public QDialog {
    Q_OBJECT

public:
    explicit FontDialog(QWidget *parent = nullptr);
    QString getSelectedFontCommand() const;

private:
    QListWidget *fontList; // List widget to display font options
};

#endif // FONTDIALOG_H
