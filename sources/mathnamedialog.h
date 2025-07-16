#ifndef MATHNAMEDIALOG_H
#define MATHNAMEDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QGroupBox>

class MathNameDialog : public QDialog {
    Q_OBJECT

public:
    explicit MathNameDialog(QWidget *parent = nullptr);
    QString getSelectedMathName() const;
    QString getLimitPlacement() const;

private:
    QComboBox *mathNameComboBox;
    QComboBox *limitPlacementComboBox;
    QPushButton *okButton;
    QPushButton *cancelButton;
};

#endif // MATHNAMEDIALOG_H
