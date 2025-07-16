#ifndef BRACKETCHOOSERDIALOG_H
#define BRACKETCHOOSERDIALOG_H

#include <QDialog>
#include <QButtonGroup>
#include <QSettings>

class BracketChooserDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BracketChooserDialog(QWidget *parent = nullptr);
    QString getLeftBracket() const;
    QString getRightBracket() const;
    QString toLaTeX(const QString &left, const QString &right) const;

private slots:
    void saveLastSelections();

private:
    QButtonGroup *leftBracketGroup;
    QButtonGroup *rightBracketGroup;

    QSettings settings;

    QString lastLeftBracket;
    QString lastRightBracket;

    void loadLastSelections();
};

#endif // BRACKETCHOOSERDIALOG_H
