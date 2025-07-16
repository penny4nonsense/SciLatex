#ifndef DECORATORDIALOG_H
#define DECORATORDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

class DecoratorDialog : public QDialog {
    Q_OBJECT

public:
    explicit DecoratorDialog(QWidget *parent = nullptr);
    QString getSelectedDecorator() const;

private:
    QListWidget *decoratorList;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QString selectedDecorator;
};

#endif // DECORATORDIALOG_H
