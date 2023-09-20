#ifndef RECORDSTORAGE_H
#define RECORDSTORAGE_H


#include <QObject>
#include "recordblackbox.h"
#include "util.h"
#include <QTime>
#include <QDate>
#include "debug.h"
#include "util.h"

class RecordStorage : public QObject
{
    Q_OBJECT
    RecordBlackBox RecMas1[RECNUM];
    RecordBlackBox RecMas2[RECNUM];
    RecordBlackBox RecMas3[RECNUM];
    RecordBlackBox RecMas4[RECNUM];

    qint16 size;
    qint16 curPos[4];

private:
    bool ISNEW1;
    bool ISNEW2;
    bool ISNEW3;
    bool ISNEW4;

    bool BLOCK1;
    bool BLOCK2;
    bool BLOCK3;
    bool BLOCK4;

    QString dataDirectoryName;

public:
    explicit RecordStorage(QObject *parent = 0,qint16 num=1,QString homeDir="");
    ~RecordStorage();

    void saveToFile(qint8 chnum); //QString filename1="b1.bd",QString filename2="b2.bd",QString filename3="b3.bd");
    void clearRec(qint8 which);
    void add(RecordBlackBox element,qint8 chan);
    RecordBlackBox get(qint16 num,qint8 chan);
    RecordBlackBox getLast(qint8 chan);
    RecordBlackBox getFirst(qint8 chan);
    qint16 getCurPos(qint8 chan);
    bool isNew(qint8 chan);




signals:

public slots:

};

#endif // RECORDSTORAGE_H
