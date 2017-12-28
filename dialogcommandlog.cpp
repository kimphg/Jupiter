#include "dialogcommandlog.h"
#include "ui_dialogcommandlog.h"

DialogCommandLog::DialogCommandLog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogCommandLog)
{
    ui->setupUi(this);
}

DialogCommandLog::~DialogCommandLog()
{
    delete ui;
}
void DialogCommandLog::AddString(QString str)
{
    if(str!=oldStr)
    {
        oldStr = str;
        ui->textBrowser->append(str);
    }
}
