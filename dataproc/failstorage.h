#ifndef FAILSTORAGE_H
#define FAILSTORAGE_H

#include <QObject>
#include <QTimer>
#include "recordblackbox.h"
//#include "failrecord.h"

struct FailPatterns
{   //                         1        2        4       8         16
    //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB3 3-SGNCNTR 4-VALMK_DIAG
    quint8 checkBytes=0;
    //какой бит проверять
    quint8 ValUSO_bit[8];   // 0 - значение 0 , 1-значение один , 5-не имеет значения
    quint8 ValMK_bit[8];    // 0 - значение 0 , 1-значение один , 5-не имеет значения
    quint8 OUTB1_bit[8];    
    quint8 ValMKDiag_bit[8];
  //  //какое значение проверяемого бита(1,0)

    // какие достоверности аналоговых проверять
    quint8 errSig[15];
    quint8 errSigValue[15];


    //какие аналоговые сигналы выводить
    quint8 checkAnalogSignals[15];

    QString failName;
    QString failSource;



    quint16 setTime=0;
    quint16 usetTime=0;



    quint8 failDeviceMask[73];//28 + 45 окно с устройствами 0 - не зажигать, 1 - желтый цвет



    quint8 num;
    quint8 failTimeConst; //
    quint8 failTimeCount;

    bool waitTimeOut;
    bool isSet = false ;
    bool isFinished = false ;
    bool isLocal=false;
};

const QString sgnName[15]={"N.11 ","N.21 ","N.31 ","N.12 ","N.22 ","N.32 ","P2.1 ","P2.2 ","P2.3 ","P2.4 ","P2.5 ","P2.6 ",
                   "P1.1 ","P1.2 ","P1.3 "};
struct FailRecord
{


    QString failName;
    qint8 attribute;
    qint8 startTime[12];
    qint8 stoptTime[12];

    bool start=false;
    quint8 failPatternNum;
    quint8 failDeviceMask[73];


};

class FailStorage : public QObject
{
    Q_OBJECT
public:
    explicit FailStorage(QObject *parent = 0,qint16 num =0,QString homeDir="" );

signals:

public:

    FailRecord failMas1[12000];
    FailRecord failMas2[12000];
    FailRecord failMas3[12000];
    FailRecord failMas4[12000];

    void saveToFileOneString(bool isLocal,QString failName );
    void saveToFile(qint8 chnum); //QString filename1="b1.bd",QString filename2="b2.bd",QString filename3="b3.bd");
    void clearRec(qint8 which);
    void initFailPatterns(void );
    void chekFailRec(RecordBlackBox *element1=NULL,RecordBlackBox *element2=NULL,RecordBlackBox *element3=NULL,RecordBlackBox *elementDiag=NULL);
    void add(quint8 chan,FailPatterns fp,RecordBlackBox *elem);

    quint8 *    getFailDeviceMas();
    QString *   getFailDeviceMess();

    FailRecord get(qint16 num,qint8 chan);
    FailRecord getLast(qint8 chan);
    FailRecord getFirst(qint8 chan);
    qint16 getCurPos(qint8 chan);

    QString getFailDir();




/*
    void saveToFile(qint8 chnum); //QString filename1="b1.bd",QString filename2="b2.bd",QString filename3="b3.bd");
    void clearRec(qint8 which);
    void add(RecordBlackBox element,qint8 chan);
    RecordBlackBox get(qint16 num,qint8 chan);
    RecordBlackBox getLast(qint8 chan);
    RecordBlackBox getFirst(qint8 chan);
    qint16 getCurPos(qint8 chan);
    bool isNew(qint8 chan);
*/

    quint8 failDeviceMas[73];
    QString failDeviceMess[73];

public slots:

    void updateTimer100mls(void);


private:

    void initFailPatternsChan1(void);
    void initFailPatternsChan2(void);
    void initFailPatternsChan3(void);
    void initFailPatternsChan4(void);
    qint16 FailPattNum;
    FailPatterns failPatterns[300];
    quint8 failTimeOutMas[300];


    qint16 size;
    qint16 curPos[4];

    QString failDirectoryName;


    QTimer * timer100mls;



    bool ISNEW1;
    bool ISNEW2;
    bool ISNEW3;
    bool ISNEW4;

    bool BLOCK1;
    bool BLOCK2;
    bool BLOCK3;
    bool BLOCK4;
};

#endif // FAILSTORAGE_H
