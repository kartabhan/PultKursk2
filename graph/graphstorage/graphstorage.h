#ifndef GRAPHSTORAGE_H
#define GRAPHSTORAGE_H


#include <QObject>

#include "graphblackbox.h"
#include "../defines.h"
#include "recordblackbox.h"

class GraphStorage : public QObject
{
    Q_OBJECT


public:
  //  StructGraphBlackBox RecMas[MAX_REC_IN_FILE];
    RecordBlackBox RecMas[MAX_REC_IN_FILE];

    qint16 size;
    qint16 curPos[3];
    qint16 numOfRecord;

private:

public:
    explicit GraphStorage(QObject *parent = 0,qint16 num=1);
    ~GraphStorage();

    void openFile(QString filename1="b1.bd");
    qint16 getSize(void);
    RecordBlackBox  getRecByNum(qint16 num);




signals:

public slots:

};

#endif // GRAPHSTORAGE_H
