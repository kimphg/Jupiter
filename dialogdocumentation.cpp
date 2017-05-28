#include "dialogdocumentation.h"
#include "ui_dialogdocumentation.h"

DialogDocumentation::DialogDocumentation(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogDocumentation)
{
    ui->setupUi(this);
}

DialogDocumentation::~DialogDocumentation()
{
    delete ui;
}
