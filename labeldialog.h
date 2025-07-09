#ifndef LABELDIALOG_H
#define LABELDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

class LabelDialog : public QDialog {
    Q_OBJECT

public:
    explicit LabelDialog(QWidget *parent = nullptr);
    QString getSelectedLabelType() const;

private:
    QPushButton *aboveButton;
    QPushButton *belowButton;
    QString selectedLabel;
};

#endif // LABELDIALOG_H
