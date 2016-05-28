#include "onexitdialog.h"
#include "ui_onexitdialog.h"

OnExitDialog::OnExitDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OnExitDialog)
{
    ui->setupUi(this);
}

OnExitDialog::~OnExitDialog()
{
    delete ui;
}

void OnExitDialog::on_buttonBox_accepted()
{
    delete this;
}
