#ifndef SHOWDATA_H
#define SHOWDATA_H

#include <QObject>
#include "recordblackbox.h"
#include "debug.h"
class ShowData : public QObject
{
    Q_OBJECT
public:
    explicit ShowData(QObject *parent = 0);
    ~ShowData();

    QString strcom1;
    QString strcom2;
    QString strcom3;
    QString strcom4;
    bool pWork1;
    bool pWork2;
    bool pWork3;
    bool pWork4;

public:
signals:
    void NewRecReadyMK1(RecordBlackBox element);
    void NewRecReadyMK2(RecordBlackBox element);
    void NewRecReadyMK3(RecordBlackBox element);
    void NewRecReadyMK4(RecordBlackBox element);
public  slots:
    void on_new_txt1(QString *data);
    void on_new_txt2(QString *data);
    void on_new_txt3(QString *data);
    void on_new_txt4(QString *data);
private:
qint16 Trans4in2 (QChar *pb);
qint16 KS (QChar *ptr, qint16 len);
};

#endif // SHOWDATA_H
