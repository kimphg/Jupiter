#ifndef DIALOGCOMMANDLOG_H
#define DIALOGCOMMANDLOG_H

#include <QDialog>

namespace Ui {
class DialogCommandLog;
}

class DialogCommandLog : public QDialog
{
    Q_OBJECT

public:
    explicit DialogCommandLog(QWidget *parent = 0);
    ~DialogCommandLog();

    void AddString(QString str);

private:
    QString oldStr;
    Ui::DialogCommandLog *ui;
};

#endif // DIALOGCOMMANDLOG_H
