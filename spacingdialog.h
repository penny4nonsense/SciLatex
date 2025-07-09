#ifndef SPACINGDIALOG_H
#define SPACINGDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

class SpacingDialog : public QDialog {
    Q_OBJECT

public:
    explicit SpacingDialog(QWidget *parent = nullptr);
    QString getSelectedSpacing() const;

private:
    QListWidget *spacingList;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QString selectedSpacing;
};

#endif // SPACINGDIALOG_H
