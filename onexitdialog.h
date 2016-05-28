#ifndef ONEXITDIALOG_H
#define ONEXITDIALOG_H

#include <QDialog>

namespace Ui {
class OnExitDialog;
}

class OnExitDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OnExitDialog(QWidget *parent = 0);
    ~OnExitDialog();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::OnExitDialog *ui;
};

#endif // ONEXITDIALOG_H
