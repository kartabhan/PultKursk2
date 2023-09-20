#include    "graph/graphstorage/graphstorage.h"
#include    <QFile>
#include    <QDataStream>

GraphStorage::GraphStorage(QObject *parent,qint16 num) : QObject(parent)
{
   /* RecMas1 = new RecordBlackBox[num];
    RecMas2 = new RecordBlackBox[num];
    RecMas3 = new RecordBlackBox[num];*/
    size = num;
    curPos[0]=0;
    curPos[1]=0;
    curPos[2]=0;
}

GraphStorage::~GraphStorage()
{

}

void GraphStorage::openFile(QString filename1)
{
   // qint16 numOfRec=0;
   QFile f(filename1);


   f.open( QIODevice::ReadWrite);
   numOfRecord = f.size()/(sizeof(RecordBlackBox));
   f.read((char *)RecMas,f.size());

 }

qint16 GraphStorage::getSize()
{
    return(numOfRecord);
}

RecordBlackBox GraphStorage::getRecByNum(qint16 num)
{
    if (num>=0 && num<=numOfRecord-1)
         return(RecMas[num]);else return(RecMas[0]);

}
