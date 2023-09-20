#ifndef AVTWINDOW_H
#define AVTWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include "siglabel.h"
#include "recordblackbox.h"

namespace Ui {
class AvtWindow;
}

class AvtWindow :  public QMainWindow
{
    Q_OBJECT

public:
    explicit AvtWindow(QWidget *parent = 0);

    void screenRefresh(RecordBlackBox elem);
    ~AvtWindow();
    qint8 busy;
    qint8 en;
protected:
    void  closeEvent(QCloseEvent *event);
private:
    Ui::AvtWindow *ui;

    QLabel *DsMas[96];
    SigLabel *sigLabelMas[68];

private slots:

    void slotExit();  
};

#endif // AVTWINDOW_H
