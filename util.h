#ifndef UTIL_H
#define UTIL_H

#include <QObject>
#include "debug.h"

#define byte quint8
#define word quint16
#define RECNUM 12000



class Util : public QObject
{
    Q_OBJECT



public:
    explicit Util(QObject *parent = 0);


static QString IntAsc(byte sm);
static byte AscInt(QString sm);

static QString Trans1In2(byte wr);
static QString Trans2In4(word wr);

static word scKS(QString * strRenamed,byte Nsm);

static QString timeCharToString(char *chMas);





signals:

public slots:

};

#endif // UTIL_H
