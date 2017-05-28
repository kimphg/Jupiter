#ifndef DIALOGDOCUMENTATION_H
#define DIALOGDOCUMENTATION_H

#include <QDialog>

namespace Ui {
class DialogDocumentation;
}

class DialogDocumentation : public QDialog
{
    Q_OBJECT

public:
     DialogDocumentation(QWidget *parent = 0);
    ~DialogDocumentation();

private:
    Ui::DialogDocumentation *ui;
};

#endif // DIALOGDOCUMENTATION_H
