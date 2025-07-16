#ifndef OPERATORDIALOG_H
#define OPERATORDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QRadioButton>
#include <QLabel>

class OperatorDialog : public QDialog {
    Q_OBJECT

public:
    explicit OperatorDialog(QWidget *parent = nullptr);

    // Getter methods for accessing private members
    QString getSelectedOperator() const;
    QString getLimitPlacement() const;
    QString getSize() const;

private slots:
    void updatePreview();

private:
    QComboBox *operatorBox;
    QRadioButton *autoLimit;
    QRadioButton *aboveBelowLimit;
    QRadioButton *rightLimit;
    QRadioButton *autoSize;
    QRadioButton *bigSize;
    QRadioButton *smallSize;
    QLabel *previewLabel;
};

#endif // OPERATORDIALOG_H
