#include    "recordstorage.h"
#include    <QFile>
#include    <QDataStream>

RecordStorage::RecordStorage(QObject *parent,qint16 num,QString homeDir) : QObject(parent)
{
   /* RecMas1 = new RecordBlackBox[num];
    RecMas2 = new RecordBlackBox[num];
    RecMas3 = new RecordBlackBox[num];*/
    size = num;
    curPos[0]=0;
    curPos[1]=0;
    curPos[2]=0;
    curPos[3]=0;


    dataDirectoryName = homeDir + "/database/";
    BLOCK1 = false;
    BLOCK2 = false;
    BLOCK3 = false;
    BLOCK4 = false;
}

RecordStorage::~RecordStorage()
{

}

void RecordStorage::saveToFile(qint8 chnum ) //QString filename1,QString filename2,QString filename3)
{
    QString fileName;
    QFile f;
    qint8 i=0;

    if (chnum == 1) {BLOCK1 =true;fileName = dataDirectoryName + "bP1_" + QDate::currentDate().toString("ddMMyy")+ "." +QString::number(chnum) + QTime::currentTime().toString("HH");}
    if (chnum == 2) {BLOCK2 =true;fileName = dataDirectoryName + "bP1_" + QDate::currentDate().toString("ddMMyy")+ "." +QString::number(chnum) + QTime::currentTime().toString("HH");}
    if (chnum == 3) {BLOCK3 =true;fileName = dataDirectoryName + "bP1_" + QDate::currentDate().toString("ddMMyy")+ "." +QString::number(chnum) + QTime::currentTime().toString("HH");}
    if (chnum == 4) {BLOCK4 =true;fileName = dataDirectoryName + "bP1D_" + QDate::currentDate().toString("ddMMyy")+ "." +QString::number(chnum) + QTime::currentTime().toString("HH");}

    f.setFileName(fileName);

    while (f.exists())
    {
     QString fileNameChange = fileName + QString::number(i);
     f.setFileName(fileNameChange );
     i++;
    }


    i = f.open( QIODevice::ReadWrite);

#ifdef DEBUGARCHIVE
    qDebug("  saveToFile chnum=%u fopenres i=%u",chnum,i);
#endif

    if (chnum == 1 ) f.write((char*)RecMas1,sizeof(RecordBlackBox)*curPos[0]);
    if (chnum == 2 ) f.write((char*)RecMas2,sizeof(RecordBlackBox)*curPos[1]);
    if (chnum == 3 ) f.write((char*)RecMas3,sizeof(RecordBlackBox)*curPos[2]);
    if (chnum == 4 ) f.write((char*)RecMas4,sizeof(RecordBlackBox)*curPos[3]);
    clearRec(chnum);

   // outStream->writeBytes((char *)&RecMas1,sizeof(RecordBlackBox)*curPos[1]);
   // outStream->writeBytes((char *)&RecMas1,sizeof(RecordBlackBox)*curPos[2]);

    f.close();

    if (chnum == 1) BLOCK1 = false;
    if (chnum == 2) BLOCK2 = false;
    if (chnum == 3) BLOCK3 = false;
    if (chnum == 4) BLOCK4 = false;
}
void RecordStorage::add(RecordBlackBox element,qint8 chan)
{
    switch(chan)
    {
    case 1:
        if (!BLOCK1)
        {
            if (curPos[0]<size)
            {RecMas1[curPos[0]]=element;curPos[0]++;ISNEW1=true;}
            else
            {saveToFile(1);
             clearRec(1);
            }
        }
        break;
    case 2:
        if (!BLOCK2)
        {
            if (curPos[1]<size){RecMas2[curPos[1]]=element;curPos[1]++;ISNEW2=true;}
            else
            {
             saveToFile(2);
             clearRec(2);
            }
        }
        break;
    case 3:
        if (!BLOCK3)
        {
            if (curPos[2]<size){RecMas3[curPos[2]]=element;curPos[2]++;ISNEW3=true;}
            else
            {
             saveToFile(3);
             clearRec(3);
            }

        }
        break;
    case 4:
        if (!BLOCK4)
        {
            if (curPos[3]<size){RecMas4[curPos[3]]=element;curPos[3]++;ISNEW4=true;}
            else
            {
             saveToFile(4);
             clearRec(4);
            }

        }
        break;
    default:
        break;
    }


}

qint16 RecordStorage::getCurPos(qint8 chan)
{
    return curPos[chan-1];
}

void RecordStorage::clearRec(qint8 which)
{
qint32 siz = RECNUM *sizeof(RecordBlackBox);
switch (which) {
    case 0:
        memset(RecMas1,0,siz);
        curPos[0]=0;
        memset(RecMas2,0,siz);
        curPos[1]=0;
        memset(RecMas3,0,siz);
        curPos[2]=0;
        memset(RecMas4,0,siz);
        curPos[3]=0;
        break;
    case 1:
        memset(RecMas1,0,siz);
        curPos[0]=0;
        break;
    case 2:
        memset(RecMas2,0,siz);
        curPos[1]=0;
        break;
   case 3:
        memset(RecMas3,0,siz);
        curPos[2]=0;
        break;
   case 4:
     memset(RecMas4,0,siz);
     curPos[3]=0;
     break;
    default:
        break;
    }

}

RecordBlackBox RecordStorage::get(qint16 num,qint8 chan)
{
    switch (chan) {
    case 1: ISNEW1=false;return RecMas1[num];

        break;
    case 2: ISNEW2=false;return RecMas2[num];

        break;
    case 3: ISNEW3=false;return RecMas3[num];

        break;
    case 4: ISNEW4=false;return RecMas4[num];

        break;
    default:
        ISNEW1=false;
        return  RecMas1[num];
        break;
    }
}
RecordBlackBox RecordStorage::getFirst(qint8 chan)
{
 return get(0,chan);
}
RecordBlackBox RecordStorage::getLast(qint8 chan)
{
    return get(curPos[chan-1]-1,chan);
}
bool RecordStorage::isNew(qint8 chan)
{

    switch (chan) {
    case 1: return ISNEW1;

        break;
    case 2: return ISNEW2;

        break;
    case 3: return ISNEW3;

        break;
    case 4: return ISNEW4;

        break;
    default:
        return ISNEW1;
        break;
    }

}

