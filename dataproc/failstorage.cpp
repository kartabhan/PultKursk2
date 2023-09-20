#include "failstorage.h"
//#include "failrecord.h"
#include "util.h"
#include <QFile>
#include <QDataStream>
#include <QTime>
#include <QTimer>
#include <QDate>
#include <QTextStream>

FailStorage::FailStorage(QObject *parent,qint16 num,QString homeDir) :
    QObject(parent)
{
    quint8 i;
    size = num;
   // size = 5;
    curPos[0]=0;
    curPos[1]=0;
    curPos[2]=0;
    curPos[3]=0;

    failDirectoryName = homeDir + "/database/";

    timer100mls = new QTimer(this);
    connect(timer100mls, SIGNAL(timeout()), this, SLOT(updateTimer100mls()));
    timer100mls->start(100);

    BLOCK1 = false;
    BLOCK2 = false;
    BLOCK3 = false;
    BLOCK4 = false;

    memset( failDeviceMas,0,sizeof(failDeviceMas));
    memset( failTimeOutMas,0,sizeof(failTimeOutMas));

    for(i=0;i<73;i++) failDeviceMess[i] ="штатная работа";

    initFailPatterns();

}





void FailStorage::add(quint8 chan,FailPatterns fp,RecordBlackBox *elem)
{
    FailRecord fr;
    QString setUnset;
    QString analogStr;
    QString addStr;
    bool analogNeed=false;
    quint8 i;

    if (fp.isLocal ) fr.attribute = 0x02;else fr.attribute = 0x01;
    fr.start = fp.isSet;
    fr.failPatternNum= fp.num;
    if (fr.start) setUnset = "установка"; else setUnset = "снятие";

    //---
    analogStr = "{";
    for(i=0;i<15;i++)
    {
        if(fp.checkAnalogSignals[i]==1)
        {   analogNeed = true;
            switch (i) {
            case 0:
                analogStr += "N.11=" +  QString::number(elem->SgnAnl[i]) + "% ";
                break;
            case 1:
                analogStr += "N.21=" +  QString::number(elem->SgnAnl[i]) + "% ";
                break;
            case 2:
                analogStr += "N.31=" +  QString::number(elem->SgnAnl[i]) + "% ";
                break;
            case 3:
                analogStr += "N.12=" +  QString::number(elem->SgnAnl[i]) + "% ";
                break;
            case 4:
                analogStr += "N.22=" +  QString::number(elem->SgnAnl[i]) + "% ";
                break;
            case 5:
                analogStr += "N.32=" +  QString::number(elem->SgnAnl[i]) + "% ";
                break;
            case 6:
                analogStr += "P2." + QString::number(i-5) + "=" + QString::number(elem->SgnAnl[i]) + "кг*с/см2 ";
                break;
            case 7:
                analogStr += "P2." + QString::number(i-5) + "=" + QString::number(elem->SgnAnl[i]) + "кг*с/см2 ";
                break;
            case 8:
                analogStr += "P2." + QString::number(i-5) + "=" + QString::number(elem->SgnAnl[i]) + "кг*с/см2 ";
                break;
            case 9:
                analogStr += "P2." + QString::number(i-5) + "=" + QString::number(elem->SgnAnl[i]) + "кг*с/см2 ";
                break;
            case 10:
                analogStr += "P2." + QString::number(i-5) + "=" + QString::number(elem->SgnAnl[i]) + "кг*с/см2 ";
                break;
            case 11:
                analogStr += "P2." + QString::number(i-5) + "=" + QString::number(elem->SgnAnl[i]) + "кг*с/см2 ";
                break;
            case 12:
                analogStr += "P1." + QString::number(i-11) + "=" + QString::number(elem->SgnAnl[i]) + "кг*с/см2 ";
                break;
            case 13:
                analogStr += "P1." + QString::number(i-11) + "=" + QString::number(elem->SgnAnl[i]) + "кг*с/см2 ";
                break;
            case 14:
                analogStr += "P1." + QString::number(i-11) + "=" + QString::number(elem->SgnAnl[i]) + "кг*с/см2 ";
                break;

            default:
                break;
            }


        }
    }
    if (analogNeed) analogStr += "}";else analogStr ="";

    addStr ="";
    if (fp.num ==69 || fp.num ==70 || fp.num ==71 || fp.num ==148 || fp.num ==149 || fp.num ==150 || fp.num ==227 || fp.num ==228 || fp.num ==229)
    {
        for(i=0;i<15;i++)
         if (elem->dsAn[i]!=0)
         {
             if ((elem->dsAn[i] & 0x01)!=0)
             {
                 addStr +=sgnName[i] + " обрыв;";
             }
             if ((elem->dsAn[i] & 0x02)!=0)
             {
                 addStr +=sgnName[i] + " ниже границы;";
             }

             if ((elem->dsAn[i] & 0x04)!=0)
             {
                 addStr +=sgnName[i] + " выше границы;";
             }

             if ((elem->dsAn[i] & 0x08)!=0)
             {
                 addStr +=sgnName[i] + " расхождение;";
             }
         }
    }

    fr.failName = QDate::currentDate().toString("dd/MM/yy").leftJustified(10,'_') + Util::timeCharToString((char *)elem->time).leftJustified(15,'_') + "MK"+ QString::number(chan).leftJustified(4,'_')  + setUnset.leftJustified(20,'_')  + fp.failSource.leftJustified(20,'_')+ fp.failName + addStr + analogStr + '\r\n';

    saveToFileOneString(fp.isLocal,fr.failName + "\r\n");

    if (fr.start)
    {
        for (i=0;i<73;i++)
        {
            if (fp.failDeviceMask[i] !=0)
            {
                failDeviceMas[i] =fp.failDeviceMask[i];

                failDeviceMess[i] = fp.failName + addStr;

            }
        }

    }else
    {
        for (i=0;i<73;i++)
        {
            if (failPatterns[fr.failPatternNum].failDeviceMask[i] !=0)
            {
                failDeviceMas[i] =0;
                failDeviceMess[i] ="штатная работа";
            }
        }

    }




    switch(chan)
    {
    case 1:
        if (!BLOCK1)
        {
            if (curPos[0]<size)
            {   failMas1[curPos[0]] = fr;
                curPos[0]++;ISNEW1=true;
            }
            else
            {
                saveToFile(1);
             clearRec(1);
            }
        }
        break;
    case 2:
        if (!BLOCK2)
        {
            if (curPos[1]<size)
            { failMas2[curPos[1]]=fr;
                curPos[1]++;ISNEW2=true;
            }
            else
            {
             //saveToFile(2);
             //clearRec(2);
            }
        }
        break;
    case 3:
        if (!BLOCK3)
        {
            if (curPos[2]<size)
            {   failMas3[curPos[2]]=fr;
                curPos[2]++;ISNEW3=true;
            }
            else
            {
             //saveToFile(3);
             //clearRec(3);
            }

        }
        break;
    case 4:
        if (!BLOCK4)
        {
            if (curPos[3]<size)
            {  failMas4[curPos[3]]=fr;
                curPos[3]++;ISNEW4=true;
            }
            else
            {
             //saveToFile(4);
            // clearRec(4);
            }

        }
        break;
    default:
        break;
    }

}
QString FailStorage::getFailDir()
{
return(failDirectoryName);
}

FailRecord FailStorage::get(qint16 num,qint8 chan)
{
    switch (chan) {
    case 1: ISNEW1=false;return failMas1[num];

        break;
    case 2: ISNEW2=false;return failMas2[num];

        break;
    case 3: ISNEW3=false;return failMas3[num];

        break;
    case 4: ISNEW4=false;return failMas4[num];

        break;
    default:
        ISNEW1=false;return failMas1[num];
        break;
    }
}
FailRecord FailStorage::getFirst(qint8 chan)
{
 return get(0,chan);
}
FailRecord FailStorage::getLast(qint8 chan)
{
    return get(curPos[chan-1]-1,chan);
}
qint16 FailStorage::getCurPos(qint8 chan)
{
    return curPos[chan-1];
}




quint8 * FailStorage::getFailDeviceMas()
{
    return((quint8*)&failDeviceMas);
}


QString * FailStorage::getFailDeviceMess()
{
    return((QString*)&failDeviceMess);
}


void FailStorage::chekFailRec(RecordBlackBox *element1,RecordBlackBox *element2,RecordBlackBox *element3,RecordBlackBox *elementDiag)
{
    quint8 i=0;
    quint8 j=0;
    quint8 k=0;
    quint8 ll=1;
    quint8 patternMatch=0;

    quint8 buf1,buf2=0;
    quint8 byt=0;
    quint8 sch=1;
    bool flag;

if (element1 !=NULL)
{
    for(i=0;i<79;i++)
    {

    if (i!=77 && i!=78 && i!=156 && i!=157 && i!=235 && i!=236) // Diag PAttern number
    {
        patternMatch=0;
        ll = 1;
        for(j=0;j<8;j++)
        {

            if (failPatterns[i].checkBytes & ll)
            {

                switch (ll) {
                case 1 :


                    byt=element1->ValUSO;
                    sch=1;
                    flag = true;
                    for(k=0;k<8;k++)
                    {
                        if ( failPatterns[i].ValUSO_bit[k] != 5)
                        {
                            buf1 = byt & sch;
                            buf2 = failPatterns[i].ValUSO_bit[k] << k;
                            if (buf1 != buf2) flag = false;

                        }
                        sch*=2;
                    }

                    if (flag)  {patternMatch+=1;}




                   /*     if ( failPatterns[i].ValUSO_value = 1)
                        {
                             if ((element1->ValUSO & failPatterns[i].ValUSO_bit) != 0) {patternMatch+=1;}

                        }else
                        {
                            if ((element1->ValUSO & failPatterns[i].ValUSO_bit) == 0) {patternMatch+=1;}
                        }*/

                    break;
                case 2 :


                    byt=element1->ValMK;
                    sch=1;
                    flag = true;
                    for(k=0;k<8;k++)
                    {
                        if ( failPatterns[i].ValMK_bit[k] != 5)
                        {
                            buf1 = byt & sch;
                            buf2 = failPatterns[i].ValMK_bit[k] << k;
                            if (buf1 != buf2) flag = false;

                        }
                        sch*=2;
                    }

                    if (flag)  {patternMatch+=1;}

                    /*if ( failPatterns[i].ValMK_value = 1)
                    {
                         if ((element1->ValMK & failPatterns[i].ValMK_bit) != 0) {patternMatch+=1;}

                    }else
                    {
                        if ((element1->ValMK & failPatterns[i].ValMK_bit) == 0) {patternMatch+=1;}
                    }*/

                    break;
                case 4 :

                    byt=element1->OutCww1[0];
                    sch=1;
                    flag=true;

                    for(k=0;k<8;k++)
                    {
                        if ( failPatterns[i].OUTB1_bit[k] != 5)
                        {
                           buf1 = byt & sch;
                           buf2 = failPatterns[i].OUTB1_bit[k] << k;
                           if (buf1 != buf2) flag = 0;

                        }
                        sch*=2;
                    }

                    if (flag)  {patternMatch+=1;}


                    break;
                case 8:
                    flag = true;
                    for(k=0;k<15;k++)
                        if ( failPatterns[i].errSigValue[k] !=0 )
                        {
                            byt=element1->dsAn[k];




                            buf1 = byt & failPatterns[i].errSigValue[k];

                            if (buf1 != failPatterns[i].errSigValue[k])  {flag = false;}


                        }

                        if (flag)  {patternMatch+=1;}


                    break;
                case 16 :

              /*      byt=;
                    sch=1;
                    flag = true;
                    for(k=0;k<8;k++)
                    {
                        if ( failPatterns[i].ValMK_bit[k] != 5)
                        {
                            buf1 = byt & sch;
                            buf2 = failPatterns[i].ValMKDiag_bit[k] << k;
                            if (buf1 != buf2) flag = false;

                        }
                        sch*=2;
                    }

                    if (flag)  {patternMatch+=1;}

*/

                    break;
                case 32 :


                    break;
                case 64 :

                    break;
                case 128 :


                    break;
                default:
                    break;

                }
            }else
            {
                patternMatch+=1;
            }
            ll*=2;
        }
        if(patternMatch == 8)
        {


                if ( failPatterns[i].isSet == false && failPatterns[i].failTimeCount == 0 && failPatterns[i].waitTimeOut == true)
                {
                    failPatterns[i].isSet = true;

                  /*  for (j=0;j<73;j++)
                    {
                        if (failPatterns[i].failDeviceMask[j] !=0)
                        {
                            failDeviceMas[j] =failPatterns[i].failDeviceMask[j];
                            failDeviceMess[j] = failPatterns[i].failName;
                        }

                    }*/
                    add(1, failPatterns[i],element1);

                }

                if (failPatterns[i].waitTimeOut ==false)
                {
                    failPatterns[i].waitTimeOut = true;
                    failPatterns[i].failTimeCount = failPatterns[i].failTimeConst;
                }


        }else
        {
            failPatterns[i].waitTimeOut =false;
            failPatterns[i].failTimeCount = 0;

            if ( failPatterns[i].isSet == true)
            {
                failPatterns[i].isSet = false;
               /* for (j=0;j<73;j++)
                {
                    if (failPatterns[i].failDeviceMask[j] !=0)
                    {
                        failDeviceMas[j] =0;
                        failDeviceMess[j] ="штатная работа";
                    }

                }*/
                add(1, failPatterns[i],element1);

            }
        }

    }// != Diag patterns number

    }

}//end of element1 procession








if (element2 !=NULL)
{
    for(i=79;i<158;i++)
    {

    if  (i!=77 && i!=78 && i!=156 && i!=157 && i!=235 && i!=236)  // Diag PAttern number
    {
        patternMatch=0;
        ll = 1;
        for(j=0;j<8;j++)
        {

            if (failPatterns[i].checkBytes & ll)
            {

                switch (ll) {
                case 1 :


                    byt=element2->ValUSO;
                    sch=1;
                    flag = true;
                    for(k=0;k<8;k++)
                    {
                        if ( failPatterns[i].ValUSO_bit[k] != 5)
                        {
                            buf1 = byt & sch;
                            buf2 = failPatterns[i].ValUSO_bit[k] << k;
                            if (buf1 != buf2) flag = false;

                        }
                        sch*=2;
                    }

                    if (flag)  {patternMatch+=1;}




                   /*     if ( failPatterns[i].ValUSO_value = 1)
                        {
                             if ((element2->ValUSO & failPatterns[i].ValUSO_bit) != 0) {patternMatch+=1;}

                        }else
                        {
                            if ((element2->ValUSO & failPatterns[i].ValUSO_bit) == 0) {patternMatch+=1;}
                        }*/

                    break;
                case 2 :


                    byt=element2->ValMK;
                    sch=1;
                    flag = true;
                    for(k=0;k<8;k++)
                    {
                        if ( failPatterns[i].ValMK_bit[k] != 5)
                        {
                            buf1 = byt & sch;
                            buf2 = failPatterns[i].ValMK_bit[k] << k;
                            if (buf1 != buf2) flag = false;

                        }
                        sch*=2;
                    }

                    if (flag)  {patternMatch+=1;}

                    /*if ( failPatterns[i].ValMK_value = 1)
                    {
                         if ((element2->ValMK & failPatterns[i].ValMK_bit) != 0) {patternMatch+=1;}

                    }else
                    {
                        if ((element2->ValMK & failPatterns[i].ValMK_bit) == 0) {patternMatch+=1;}
                    }*/

                    break;
                case 4 :

                    byt=element2->OutCww1[0];
                    sch=1;
                    flag=true;

                    for(k=0;k<8;k++)
                    {
                        if ( failPatterns[i].OUTB1_bit[k] != 5)
                        {
                           buf1 = byt & sch;
                           buf2 = failPatterns[i].OUTB1_bit[k] << k;
                           if (buf1 != buf2) flag = 0;

                        }
                        sch*=2;
                    }

                    if (flag)  {patternMatch+=1;}


                    break;
                case 8:
                    flag = true;
                    for(k=0;k<15;k++)
                        if ( failPatterns[i].errSigValue[k] !=0 )
                        {
                            byt=element2->dsAn[k];




                            buf1 = byt & failPatterns[i].errSigValue[k];

                            if (buf1 != failPatterns[i].errSigValue[k])  {flag = false;}


                        }

                        if (flag)  {patternMatch+=1;}


                    break;
                case 16 :

              /*      byt=;
                    sch=1;
                    flag = true;
                    for(k=0;k<8;k++)
                    {
                        if ( failPatterns[i].ValMK_bit[k] != 5)
                        {
                            buf1 = byt & sch;
                            buf2 = failPatterns[i].ValMKDiag_bit[k] << k;
                            if (buf1 != buf2) flag = false;

                        }
                        sch*=2;
                    }

                    if (flag)  {patternMatch+=1;}

*/

                    break;
                case 32 :


                    break;
                case 64 :

                    break;
                case 128 :


                    break;
                default:
                    break;

                }
            }else
            {
                patternMatch+=1;
            }
            ll*=2;
        }
        if(patternMatch == 8)
        {


                if ( failPatterns[i].isSet == false && failPatterns[i].failTimeCount == 0 && failPatterns[i].waitTimeOut == true)
                {
                    failPatterns[i].isSet = true;

                  /*  for (j=0;j<73;j++)
                    {
                        if (failPatterns[i].failDeviceMask[j] !=0)
                        {
                            failDeviceMas[j] =failPatterns[i].failDeviceMask[j];
                            failDeviceMess[j] = failPatterns[i].failName;
                        }

                    }*/
                    add(2, failPatterns[i],element2);

                }

                if (failPatterns[i].waitTimeOut ==false)
                {
                    failPatterns[i].waitTimeOut = true;
                    failPatterns[i].failTimeCount = failPatterns[i].failTimeConst;
                }


        }else
        {
            failPatterns[i].waitTimeOut =false;
            failPatterns[i].failTimeCount = 0;

            if ( failPatterns[i].isSet == true)
            {
                failPatterns[i].isSet = false;
               /* for (j=0;j<73;j++)
                {
                    if (failPatterns[i].failDeviceMask[j] !=0)
                    {
                        failDeviceMas[j] =0;
                        failDeviceMess[j] ="штатная работа";
                    }

                }*/
                add(2, failPatterns[i],element2);

            }
        }

    }// != Diag patterns number

    }

}//end of element2 procession





if (element3 !=NULL)
{
    for(i=158;i<237;i++)
    {

    if (i!=77 && i!=78 && i!=156 && i!=157 && i!=235 && i!=236) // Diag PAttern number
    {
        patternMatch=0;
        ll = 1;
        for(j=0;j<8;j++)
        {

            if (failPatterns[i].checkBytes & ll)
            {

                switch (ll) {
                case 1 :


                    byt=element3->ValUSO;
                    sch=1;
                    flag = true;
                    for(k=0;k<8;k++)
                    {
                        if ( failPatterns[i].ValUSO_bit[k] != 5)
                        {
                            buf1 = byt & sch;
                            buf2 = failPatterns[i].ValUSO_bit[k] << k;
                            if (buf1 != buf2) flag = false;

                        }
                        sch*=2;
                    }

                    if (flag)  {patternMatch+=1;}




                   /*     if ( failPatterns[i].ValUSO_value = 1)
                        {
                             if ((element3->ValUSO & failPatterns[i].ValUSO_bit) != 0) {patternMatch+=1;}

                        }else
                        {
                            if ((element3->ValUSO & failPatterns[i].ValUSO_bit) == 0) {patternMatch+=1;}
                        }*/

                    break;
                case 2 :


                    byt=element3->ValMK;
                    sch=1;
                    flag = true;
                    for(k=0;k<8;k++)
                    {
                        if ( failPatterns[i].ValMK_bit[k] != 5)
                        {
                            buf1 = byt & sch;
                            buf2 = failPatterns[i].ValMK_bit[k] << k;
                            if (buf1 != buf2) flag = false;

                        }
                        sch*=2;
                    }

                    if (flag)  {patternMatch+=1;}

                    /*if ( failPatterns[i].ValMK_value = 1)
                    {
                         if ((element3->ValMK & failPatterns[i].ValMK_bit) != 0) {patternMatch+=1;}

                    }else
                    {
                        if ((element3->ValMK & failPatterns[i].ValMK_bit) == 0) {patternMatch+=1;}
                    }*/

                    break;
                case 4 :

                    byt=element3->OutCww1[0];
                    sch=1;
                    flag=true;

                    for(k=0;k<8;k++)
                    {
                        if ( failPatterns[i].OUTB1_bit[k] != 5)
                        {
                           buf1 = byt & sch;
                           buf2 = failPatterns[i].OUTB1_bit[k] << k;
                           if (buf1 != buf2) flag = 0;

                        }
                        sch*=2;
                    }

                    if (flag)  {patternMatch+=1;}


                    break;
                case 8:
                    flag = true;
                    for(k=0;k<15;k++)
                        if ( failPatterns[i].errSigValue[k] !=0 )
                        {
                            byt=element3->dsAn[k];




                            buf1 = byt & failPatterns[i].errSigValue[k];

                            if (buf1 != failPatterns[i].errSigValue[k])  {flag = false;}


                        }

                        if (flag)  {patternMatch+=1;}


                    break;
                case 16 :

              /*      byt=;
                    sch=1;
                    flag = true;
                    for(k=0;k<8;k++)
                    {
                        if ( failPatterns[i].ValMK_bit[k] != 5)
                        {
                            buf1 = byt & sch;
                            buf2 = failPatterns[i].ValMKDiag_bit[k] << k;
                            if (buf1 != buf2) flag = false;

                        }
                        sch*=2;
                    }

                    if (flag)  {patternMatch+=1;}

*/

                    break;
                case 32 :


                    break;
                case 64 :

                    break;
                case 128 :


                    break;
                default:
                    break;

                }
            }else
            {
                patternMatch+=1;
            }
            ll*=2;
        }
        if(patternMatch == 8)
        {


                if ( failPatterns[i].isSet == false && failPatterns[i].failTimeCount == 0 && failPatterns[i].waitTimeOut == true)
                {
                    failPatterns[i].isSet = true;

                  /*  for (j=0;j<73;j++)
                    {
                        if (failPatterns[i].failDeviceMask[j] !=0)
                        {
                            failDeviceMas[j] =failPatterns[i].failDeviceMask[j];
                            failDeviceMess[j] = failPatterns[i].failName;
                        }

                    }*/
                    add(3, failPatterns[i],element3);

                }

                if (failPatterns[i].waitTimeOut ==false)
                {
                    failPatterns[i].waitTimeOut = true;
                    failPatterns[i].failTimeCount = failPatterns[i].failTimeConst;
                }


        }else
        {
            failPatterns[i].waitTimeOut =false;
            failPatterns[i].failTimeCount = 0;

            if ( failPatterns[i].isSet == true)
            {
                failPatterns[i].isSet = false;
               /* for (j=0;j<73;j++)
                {
                    if (failPatterns[i].failDeviceMask[j] !=0)
                    {
                        failDeviceMas[j] =0;
                        failDeviceMess[j] ="штатная работа";
                    }

                }*/
                add(3, failPatterns[i],element3);

            }
        }

    }// != Diag patterns number

    }

}//end of element3 procession



if (elementDiag !=NULL)
{
    for(i=77;i<79;i++)
    {


        patternMatch=0;
        ll = 1;
        for(j=0;j<8;j++)
        {

            if (failPatterns[i].checkBytes & ll)
            {

                switch (ll) {

                case 16 :

                    byt=elementDiag->ValMK;
                    sch=1;
                    flag = true;
                    for(k=0;k<8;k++)
                    {
                        if ( failPatterns[i].ValMKDiag_bit[k] != 5)
                        {
                            buf1 = byt & sch;
                            buf2 = failPatterns[i].ValMKDiag_bit[k] << k;
                            if (buf1 != buf2) flag = false;

                        }
                        sch*=2;
                    }

                    if (flag)  {patternMatch+=1;}



                    break;
                case 32 :


                    break;
                case 64 :

                    break;
                case 128 :


                    break;
                default:
                    break;

                }
            }else
            {
                patternMatch+=1;
            }
            ll*=2;
        }
        if(patternMatch == 8)
        {


                if ( failPatterns[i].isSet == false && failPatterns[i].failTimeCount == 0 && failPatterns[i].waitTimeOut == true)
                {
                    failPatterns[i].isSet = true;

                  /*  for (j=0;j<73;j++)
                    {
                        if (failPatterns[i].failDeviceMask[j] !=0)
                        {
                            failDeviceMas[j] =failPatterns[i].failDeviceMask[j];
                            failDeviceMess[j] = failPatterns[i].failName;
                        }

                    }*/
                    add(1, failPatterns[i],elementDiag);

                }

                if (failPatterns[i].waitTimeOut ==false)
                {
                    failPatterns[i].waitTimeOut = true;
                    failPatterns[i].failTimeCount = failPatterns[i].failTimeConst;
                }


        }else
        {
            failPatterns[i].waitTimeOut =false;
            failPatterns[i].failTimeCount = 0;

            if ( failPatterns[i].isSet == true)
            {
                failPatterns[i].isSet = false;
               /* for (j=0;j<73;j++)
                {
                    if (failPatterns[i].failDeviceMask[j] !=0)
                    {
                        failDeviceMas[j] =0;
                        failDeviceMess[j] ="штатная работа";
                    }

                }*/
                add(4, failPatterns[i],elementDiag);

            }
        }


    }

}//end of elementDiag procession



}

void FailStorage::clearRec(qint8 which)
{
qint32 siz = RECNUM *sizeof(FailRecord);
switch (which) {
    case 0:
        memset(failMas1,0,siz);
        curPos[0]=0;
        memset(failMas2,0,siz);
        curPos[1]=0;
        memset(failMas3,0,siz);
        curPos[2]=0;
        memset(failMas4,0,siz);
        curPos[3]=0;
        break;
    case 1:
        //memset(failMas1,0,siz);
        curPos[0]=0;
        break;
    case 2:
        memset(failMas2,0,siz);
        curPos[1]=0;
        break;
   case 3:
        memset(failMas3,0,siz);
        curPos[2]=0;
        break;
   case 4:
     memset(failMas4,0,siz);
     curPos[3]=0;
     break;
    default:
        break;
    }

}


void FailStorage::saveToFileOneString(bool isLocal,QString failName ) //QString filename1,QString filename2,QString filename3)
{
    QString fileNameMainLocal = failDirectoryName + "FailLoc.txt";
    QString fileNameMainCommon = failDirectoryName + "FailCom.txt";
    QString fileNameBackup = failDirectoryName + "Fail_back.txt";

    QFile f;
    QFile f_b(fileNameBackup);

    QTextStream wrStream(&f);
    QTextStream wrStreamBack(&f_b);
    qint8 i=0;

 if (isLocal)
 {
     f.setFileName(fileNameMainLocal);
     f_b.setFileName(fileNameBackup);
 }else
 {
     f.setFileName(fileNameMainCommon);
     f_b.setFileName(fileNameBackup);
 }
#ifdef DEBUGARCHIVE
    qDebug("  saveToFileOneString ");
#endif


    i = f.open( QIODevice::ReadWrite | QIODevice::Text | QIODevice::Append);
    wrStream <<  (failName);
    f.close();


}

void FailStorage::saveToFile(qint8 chnum ) //QString filename1,QString filename2,QString filename3)
{
   // QString fileName = failDirectoryName + "FailP2_" + QDate::currentDate().toString("ddMMyy")+ "." +QString::number(chnum) + QTime::currentTime().toString("HH");
    QString fileNameMain = failDirectoryName + "Fail.txt";
    QString fileNameBackup = failDirectoryName + "Fail_back.txt";

    QFile f(fileNameMain);
    QFile f_b(fileNameBackup);

    QTextStream wrStream(&f);
    QTextStream wrStreamBack(&f_b);
    qint8 i=0;

    if (chnum == 1) BLOCK1 =true;
    if (chnum == 2) BLOCK2 =true;
    if (chnum == 3) BLOCK3 =true;
    if (chnum == 4) BLOCK4 =true;

   /* while (f.exists())
    {
     QString fileNameChange = fileName + QString::number(i);
     f.setFileName(fileNameChange );
     i++;
    }*/


    i = f.open( QIODevice::ReadWrite | QIODevice::Text | QIODevice::Append);

#ifdef DEBUGARCHIVE
    qDebug("  saveToFile chnum=%u fopenres i=%u",chnum,i);
#endif

    if (chnum == 1 )
    {
      for (i=0;i<curPos[0];i++)
      {
        wrStream <<  (failMas1[i].failName);
      }
    }


    if (chnum == 2 ) f.write((char*)failMas2,sizeof(FailRecord)*curPos[1]);
    if (chnum == 3 ) f.write((char*)failMas3,sizeof(FailRecord)*curPos[2]);
    if (chnum == 4 ) f.write((char*)failMas4,sizeof(FailRecord)*curPos[3]);
    clearRec(chnum);

   // outStream->writeBytes((char *)&RecMas1,sizeof(RecordBlackBox)*curPos[1]);
   // outStream->writeBytes((char *)&RecMas1,sizeof(RecordBlackBox)*curPos[2]);

    f.close();

    if (chnum == 1) BLOCK1 = false;
    if (chnum == 2) BLOCK2 = false;
    if (chnum == 3) BLOCK3 = false;
    if (chnum == 4) BLOCK4 = false;
}

void FailStorage::updateTimer100mls(void)
{
 quint8 i;

    for(i=0;i<FailPattNum;i++)
    {

        if (failPatterns[i].failTimeCount > 0)
        {
            failPatterns[i].failTimeCount-=1;
        }else
        {
            failPatterns[i].failTimeCount = 0;
        }
        /*if (failPatterns[i].isSet)
        {
            if (failTimeOutMas[i]>0) failTimeOutMas[i]-=1;
        }*/
    }


}

void FailStorage::initFailPatternsChan1(void)
{
     quint8 curPattNum=0;
    //------------------------------------------------Test ---------------------------------------------------------------

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  ОШибка файла с параметрами
        curPattNum=0;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x01;// 0x06  0-VALUSO 2-VALMK  4-OUTB3
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);



        memset(failPatterns[curPattNum].OUTB1_bit,5,8);

        memset(failPatterns[curPattNum].ValUSO_bit,5,8);
        failPatterns[curPattNum].ValUSO_bit[1] = 1;

        memset(failPatterns[curPattNum].errSig,0,15);


        memset(failPatterns[curPattNum].errSigValue,0,15);


        failPatterns[curPattNum].failName = "Ошибка файла с параметрами(Выдать ИСХ.)";
        failPatterns[curPattNum].failSource = "ПМИ07Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[0]=2;


        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  Не прошел тест ПМИ07Р
        curPattNum=1;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x01;// 0x06  0-VALUSO 2-VALMK  4-OUTB3
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);



        memset(failPatterns[curPattNum].OUTB1_bit,5,8);

        memset(failPatterns[curPattNum].ValUSO_bit,5,8);
        failPatterns[curPattNum].ValUSO_bit[2] = 1;


        memset(failPatterns[curPattNum].errSig,0,15);


        memset(failPatterns[curPattNum].errSigValue,0,15);


        failPatterns[curPattNum].failName = "Не прошел тест процессора ПМИ07Р(Выдать ИСХ.)";
        failPatterns[curPattNum].failSource = "ПМИ07Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[0]=2;


        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  Не прошел тест АВВ14Р
        curPattNum=2;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x01;// 0x06  0-VALUSO 2-VALMK  4-OUTB3
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);



        memset(failPatterns[curPattNum].OUTB1_bit,5,8);

        memset(failPatterns[curPattNum].ValUSO_bit,5,8);
        failPatterns[curPattNum].ValUSO_bit[3] = 1;

        memset(failPatterns[curPattNum].errSig,0,15);


        memset(failPatterns[curPattNum].errSigValue,0,15);


        failPatterns[curPattNum].failName = "Не прошел тест АВВ14Р(Выдать ИСХ.)";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=2;


        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  Не прошел тест 1ЦВВ15М
        curPattNum=3;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x01;// 0x06  0-VALUSO 2-VALMK  4-OUTB3
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);



        memset(failPatterns[curPattNum].OUTB1_bit,5,8);

        memset(failPatterns[curPattNum].ValUSO_bit,5,8);
        failPatterns[curPattNum].ValUSO_bit[4] = 1;

        memset(failPatterns[curPattNum].errSig,0,15);


        memset(failPatterns[curPattNum].errSigValue,0,15);


        failPatterns[curPattNum].failName = "Не прошел тест 1ЦВВ15М(Выдать ИСХ.)";
        failPatterns[curPattNum].failSource = "ЦВВ15М";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[2]=2;


        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  Не прошел тест 2ЦВВ15М
        curPattNum=4;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x01;// 0x06  0-VALUSO 2-VALMK  4-OUTB3
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);



        memset(failPatterns[curPattNum].OUTB1_bit,5,8);

        memset(failPatterns[curPattNum].ValUSO_bit,5,8);
        failPatterns[curPattNum].ValUSO_bit[5] = 1;

        memset(failPatterns[curPattNum].errSig,0,15);


        memset(failPatterns[curPattNum].errSigValue,0,15);


        failPatterns[curPattNum].failName = "Не прошел тест 2ЦВВ15М(Выдать ИСХ.)";
        failPatterns[curPattNum].failSource = "ЦВВ15М";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[3]=2;


        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  Не прошел тест 3ЦВВ15М
        curPattNum=5;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x01;// 0x06  0-VALUSO 2-VALMK  4-OUTB3
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);



        memset(failPatterns[curPattNum].OUTB1_bit,5,8);

        memset(failPatterns[curPattNum].ValUSO_bit,5,8);
        failPatterns[curPattNum].ValUSO_bit[6] = 1;

        memset(failPatterns[curPattNum].errSig,0,15);


        memset(failPatterns[curPattNum].errSigValue,0,15);


        failPatterns[curPattNum].failName = "Не прошел тест 3ЦВВ15М(Выдать ИСХ.)";
        failPatterns[curPattNum].failSource = "ЦВВ15М";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[4]=2;


        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  Нет связи по RS485
        curPattNum=6;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x02;// 0x06  0-VALUSO 2-VALMK  4-OUTB3
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);
        failPatterns[curPattNum].ValMK_bit[0] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);

        memset(failPatterns[curPattNum].ValUSO_bit,5,8);


        memset(failPatterns[curPattNum].errSig,0,15);


        memset(failPatterns[curPattNum].errSigValue,0,15);


        failPatterns[curPattNum].failName = "Нет связи по R485(Выдать ИСХ.)";
        failPatterns[curPattNum].failSource = "ПМИ07Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[0]=2;


        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  Останов канала
        curPattNum = 7;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB3 3-SGNCNTR

        failPatterns[curPattNum].checkBytes = 0x02;//  0-VALUSO 2-VALMK  4-OUTB3 8-ErrNo
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);
        failPatterns[curPattNum].ValMK_bit[1] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSig,0,15);


        failPatterns[curPattNum].failName = "Останов канала 1";
        failPatterns[curPattNum].failSource = "ПМИ07Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[0]=1;



        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);



        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =1; // 1*100mls

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  нет приема сигнала
        curPattNum = 8;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB3 3-SGNCNTR

        failPatterns[curPattNum].checkBytes = 0x02;//  0-VALUSO 2-VALMK  4-OUTB3 8-ErrNo
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);
        failPatterns[curPattNum].ValMK_bit[2] = 1;
        failPatterns[curPattNum].ValMK_bit[3] = 1;

        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSig,0,15);


        failPatterns[curPattNum].failName = "Нет приема сигнала";
        failPatterns[curPattNum].failSource = "ПМИ07Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[0]=1;



        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);



        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =1; // 1*100mls

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;









    //------------------------------------------------N---------------------------------------------------------------



        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая нейтронная мощность меньше границы (6 неисправностей)
        curPattNum= 9 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;

        memset(failPatterns[curPattNum].OUTB1_bit,5,8);

        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[0] = 0x02;


        failPatterns[curPattNum].failName = "Сигнал N.11 ниже допустимой границы";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[28]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[0]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая нейтронная мощность меньше границы (6 неисправностей)
        curPattNum= 10 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[1] = 0x02;


        failPatterns[curPattNum].failName = "Сигнал N.21 ниже допустимой границы";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[29]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[1]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая нейтронная мощность меньше границы (6 неисправностей)
        curPattNum= 11;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[2] = 0x02;


        failPatterns[curPattNum].failName = "Сигнал N.31 ниже допустимой границы";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[30]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[2]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая нейтронная мощность меньше границы (6 неисправностей)
        curPattNum= 12 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[3] = 0x02;


        failPatterns[curPattNum].failName = "Сигнал N.12 ниже допустимой границы";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[31]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[3]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая нейтронная мощность меньше границы (6 неисправностей)
        curPattNum= 13 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[4] = 0x02;


        failPatterns[curPattNum].failName = "Сигнал N.22 ниже допустимой границы";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[32]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[4]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая нейтронная мощность меньше границы (6 неисправностей)
        curPattNum= 14 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[5] = 0x02;


        failPatterns[curPattNum].failName = "Сигнал N.32 ниже допустимой границы";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[33]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[5]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;






        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая нейтронная мощность выше границы (6 неисправностей)
        curPattNum= 15 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[0] = 0x04;


        failPatterns[curPattNum].failName = "Сигнал N.11 выше допустимой границы";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[28]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[0]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая нейтронная мощность выше границы (6 неисправностей)
        curPattNum= 16 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[1] = 0x04;


        failPatterns[curPattNum].failName = "Сигнал N.21 выше допустимой границы";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[29]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[1]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая нейтронная мощность выше границы (6 неисправностей)
        curPattNum= 17 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[2] = 0x04;


        failPatterns[curPattNum].failName = "Сигнал N.31 выше допустимой границы";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[30]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[2]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая нейтронная мощность выше границы (6 неисправностей)
        curPattNum= 18;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[3] = 0x04;


        failPatterns[curPattNum].failName = "Сигнал N.12 выше допустимой границы";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[31]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[3]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая нейтронная мощность выше границы (6 неисправностей)
        curPattNum= 19 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[4] = 0x04;




        failPatterns[curPattNum].failName = "Сигнал N.22 выше допустимой границы";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[32]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[4]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая нейтронная мощность выше границы (6 неисправностей)
        curPattNum= 20 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[5] = 0x04;


        failPatterns[curPattNum].failName = "Сигнал N.32 выше допустимой границы";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[33]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[5]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;




        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая нейтронная мощность обрыв линии (6 неисправностей)
        curPattNum= 21 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[0] = 0x01;

        failPatterns[curPattNum].failName = "Сигнал N.11, обрыв линии";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[28]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[0]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;


        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая нейтронная мощность обрыв линии (6 неисправностей)
        curPattNum= 22 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[1] = 0x01;

        failPatterns[curPattNum].failName = "Сигнал N.21, обрыв линии";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[29]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[1]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;


        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая нейтронная мощность обрыв линии (6 неисправностей)
        curPattNum= 23 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[2] = 0x01;

        failPatterns[curPattNum].failName = "Сигнал N.31, обрыв линии";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[30]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[2]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая нейтронная мощность обрыв линии (6 неисправностей)
        curPattNum= 24 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[3] = 0x01;

        failPatterns[curPattNum].failName = "Сигнал N.12, обрыв линии";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[31]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[3]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая нейтронная мощность обрыв линии (6 неисправностей)
        curPattNum= 25 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[4] = 0x01;

        failPatterns[curPattNum].failName = "Сигнал N.22, обрыв линии";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[32]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[4]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая нейтронная мощность обрыв линии (6 неисправностей)
        curPattNum= 26 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[5] = 0x01;

        failPatterns[curPattNum].failName = "Сигнал N.32, обрыв линии";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[33]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[5]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;




        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая нейтронная мощность рассогласование (6 неисправностей)
        curPattNum= 27;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[0] = 0x08;

        failPatterns[curPattNum].failName = "Сигнал N.11, рассогласование сигналов";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[28]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[0]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая нейтронная мощность рассогласование (6 неисправностей)
        curPattNum= 28;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[1] = 0x08;

        failPatterns[curPattNum].failName = "Сигнал N.21, рассогласование сигналов";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[29]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[1]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая нейтронная мощность рассогласование (6 неисправностей)
        curPattNum= 29;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[2] = 0x08;

        failPatterns[curPattNum].failName = "Сигнал N.31, рассогласование сигналов";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[30]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[2]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая нейтронная мощность рассогласование (6 неисправностей)
        curPattNum= 30;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[3] = 0x08;

        failPatterns[curPattNum].failName = "Сигнал N.12, рассогласование сигналов";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[31]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[3]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая нейтронная мощность рассогласование (6 неисправностей)
        curPattNum= 31;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[4] = 0x08;

        failPatterns[curPattNum].failName = "Сигнал N.22, рассогласование сигналов";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[32]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[4]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая нейтронная мощность рассогласование (6 неисправностей)
        curPattNum= 32;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[5] = 0x08;

        failPatterns[curPattNum].failName = "Сигнал N.32, рассогласование сигналов";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[33]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[5]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;
















    //------------------------------------------------P2---------------------------------------------------------------
    //------------------------------------------------P2---------------------------------------------------------------
    //------------------------------------------------P2---------------------------------------------------------------
    //------------------------------------------------P2---------------------------------------------------------------
    //------------------------------------------------P2---------------------------------------------------------------


        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление ГПК меньше границы (6 неисправностей)
        curPattNum= 33 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;

        memset(failPatterns[curPattNum].OUTB1_bit,5,8);

        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[6] = 0x02;


        failPatterns[curPattNum].failName = "Сигнал P2.1 ниже допустимой границы";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[34]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[6]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление ГПК меньше границы (6 неисправностей)
        curPattNum= 34 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[7] = 0x02;


        failPatterns[curPattNum].failName = "Сигнал P2.2 ниже допустимой границы";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[35]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[7]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление ГПК меньше границы (6 неисправностей)
        curPattNum= 35;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[8] = 0x02;


        failPatterns[curPattNum].failName = "Сигнал P2.3 ниже допустимой границы";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[36]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[8]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление ГПК меньше границы (6 неисправностей)
        curPattNum= 36 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[9] = 0x02;


        failPatterns[curPattNum].failName = "Сигнал P2.4 ниже допустимой границы";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[37]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[9]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление ГПК меньше границы (6 неисправностей)
        curPattNum= 37 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[10] = 0x02;


        failPatterns[curPattNum].failName = "Сигнал P2.5 ниже допустимой границы";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[38]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[10]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление ГПК меньше границы (6 неисправностей)
        curPattNum= 38 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[11] = 0x02;


        failPatterns[curPattNum].failName = "Сигнал P2.6 ниже допустимой границы";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[39]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[11]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;






        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление ГПК выше границы (6 неисправностей)
        curPattNum= 39 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[6] = 0x04;


        failPatterns[curPattNum].failName = "Сигнал P2.1 выше допустимой границы";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[34]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[6]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление ГПК выше границы (6 неисправностей)
        curPattNum= 40 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[7] = 0x04;


        failPatterns[curPattNum].failName = "Сигнал P2.2 выше допустимой границы";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[35]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[7]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление ГПК выше границы (6 неисправностей)
        curPattNum= 41;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[8] = 0x04;


        failPatterns[curPattNum].failName = "Сигнал P2.3 выше допустимой границы";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[36]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[8]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление ГПК выше границы (6 неисправностей)
        curPattNum= 42;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[9] = 0x04;


        failPatterns[curPattNum].failName = "Сигнал P2.4 выше допустимой границы";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[37]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[9]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление ГПК выше границы (6 неисправностей)
        curPattNum= 43 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[10] = 0x04;




        failPatterns[curPattNum].failName = "Сигнал P2.5 выше допустимой границы";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[38]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[10]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление ГПК выше границы (6 неисправностей)
        curPattNum= 44 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[11] = 0x04;


        failPatterns[curPattNum].failName = "Сигнал P2.6 выше допустимой границы";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[39]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[11]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;




        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление ГПК обрыв линии (6 неисправностей)
        curPattNum= 45 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[6] = 0x01;

        failPatterns[curPattNum].failName = "Сигнал P2.1, обрыв линии";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[34]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[6]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;


        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление ГПК обрыв линии (6 неисправностей)
        curPattNum= 46 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[7] = 0x01;

        failPatterns[curPattNum].failName = "Сигнал P2.2, обрыв линии";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[35]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[7]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;


        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление ГПК обрыв линии (6 неисправностей)
        curPattNum= 47 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[8] = 0x01;

        failPatterns[curPattNum].failName = "Сигнал P2.3, обрыв линии";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[36]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[8]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление ГПК обрыв линии (6 неисправностей)
        curPattNum= 48 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[9] = 0x01;

        failPatterns[curPattNum].failName = "Сигнал P2.4, обрыв линии";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[37]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[9]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление ГПК обрыв линии (6 неисправностей)
        curPattNum= 49 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[10] = 0x01;

        failPatterns[curPattNum].failName = "Сигнал P2.5, обрыв линии";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[38]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[10]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление ГПК обрыв линии (6 неисправностей)
        curPattNum= 50 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[11] = 0x01;

        failPatterns[curPattNum].failName = "Сигнал P2.6, обрыв линии";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[39]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[11]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;




        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление ГПК рассогласование (6 неисправностей)
        curPattNum= 51;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[6] = 0x08;

        failPatterns[curPattNum].failName = "Сигнал P2.1, рассогласование сигналов";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[34]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[6]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление ГПК рассогласование (6 неисправностей)
        curPattNum= 52;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[7] = 0x08;

        failPatterns[curPattNum].failName = "Сигнал P2.2, рассогласование сигналов";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[35]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[7]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление ГПК рассогласование (6 неисправностей)
        curPattNum= 53;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[8] = 0x08;

        failPatterns[curPattNum].failName = "Сигнал P2.3, рассогласование сигналов";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[36]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[8]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление ГПК рассогласование (6 неисправностей)
        curPattNum= 54;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[9] = 0x08;

        failPatterns[curPattNum].failName = "Сигнал P2.4, рассогласование сигналов";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[37]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[9]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление ГПК рассогласование (6 неисправностей)
        curPattNum= 55;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[10] = 0x08;

        failPatterns[curPattNum].failName = "Сигнал P2.5, рассогласование сигналов";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[38]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[10]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление ГПК рассогласование (6 неисправностей)
        curPattNum= 56;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[11] = 0x08;

        failPatterns[curPattNum].failName = "Сигнал P2.6, рассогласование сигналов";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[39]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[11]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;


















    //------------------------------------------------P1---------------------------------------------------------------
    //------------------------------------------------P1---------------------------------------------------------------
    //------------------------------------------------P1---------------------------------------------------------------
    //------------------------------------------------P1---------------------------------------------------------------

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление в первом контуре меньше границы (3 неисправностей)
        curPattNum= 57 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;

        memset(failPatterns[curPattNum].OUTB1_bit,5,8);

        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[12] = 0x02;


        failPatterns[curPattNum].failName = "Сигнал P1.1 ниже допустимой границы";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[40]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[12]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление в первом контуре меньше границы (3 неисправностей)
        curPattNum= 58 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[13] = 0x02;


        failPatterns[curPattNum].failName = "Сигнал P1.2 ниже допустимой границы";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[41]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[13]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление в первом контуре меньше границы (3 неисправностей)
        curPattNum= 59;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[14] = 0x02;


        failPatterns[curPattNum].failName = "Сигнал P1.3 ниже допустимой границы";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[42]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[14]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;







        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление в первом контуре выше границы (3 неисправностей)
        curPattNum= 60 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[12] = 0x04;


        failPatterns[curPattNum].failName = "Сигнал P1.1 выше допустимой границы";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[40]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[12]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление в первом контуре выше границы (3 неисправностей)
        curPattNum= 61 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[13] = 0x04;


        failPatterns[curPattNum].failName = "Сигнал P1.2 выше допустимой границы";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[41]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[13]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление в первом контуре выше границы (3 неисправностей)
        curPattNum= 62;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[14] = 0x04;


        failPatterns[curPattNum].failName = "Сигнал P1.3 выше допустимой границы";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[42]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[14]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;




        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление в первом контуре обрыв линии (3 неисправностей)
        curPattNum= 63 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[12] = 0x01;

        failPatterns[curPattNum].failName = "Сигнал P1.1, обрыв линии";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[40]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[12]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;


        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление в первом контуре обрыв линии (3 неисправностей)
        curPattNum= 64 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[13] = 0x01;

        failPatterns[curPattNum].failName = "Сигнал P1.2, обрыв линии";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[41]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[13]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;


        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление в первом контуре обрыв линии (3 неисправностей)
        curPattNum= 65 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[14] = 0x01;

        failPatterns[curPattNum].failName = "Сигнал P1.3, обрыв линии";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[42]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[14]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;




        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление в первом контуре рассогласование (3 неисправностей)
        curPattNum= 66;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[12] = 0x08;

        failPatterns[curPattNum].failName = "Сигнал P1.1, рассогласование сигналов";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[40]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[12]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление в первом контуре рассогласование (3 неисправностей)
        curPattNum= 67;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[13] = 0x08;

        failPatterns[curPattNum].failName = "Сигнал P1.2, рассогласование сигналов";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[41]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[13]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  каждая давление в первом контуре рассогласование (3 неисправностей)
        curPattNum= 68;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = true;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);
        failPatterns[curPattNum].errSigValue[14] = 0x08;

        failPatterns[curPattNum].failName = "Сигнал P1.3, рассогласование сигналов";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=1;
        failPatterns[curPattNum].failDeviceMask[42]=1;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[14]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;




    //------------------------------------------------analoginvalid---------------------------------------------------------------
    //------------------------------------------------analoginvalid---------------------------------------------------------------
    //------------------------------------------------analoginvalid---------------------------------------------------------------
    //------------------------------------------------analoginvalid---------------------------------------------------------------
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  недостоверность N
        curPattNum= 69 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x04;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = false;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);
        failPatterns[curPattNum].OUTB1_bit[0] = 0;

        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);


        failPatterns[curPattNum].failName = "Сигнал N недостоверен";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=2;
        failPatterns[curPattNum].failDeviceMask[28]=2;
        failPatterns[curPattNum].failDeviceMask[29]=2;
        failPatterns[curPattNum].failDeviceMask[30]=2;
        failPatterns[curPattNum].failDeviceMask[31]=2;
        failPatterns[curPattNum].failDeviceMask[32]=2;
        failPatterns[curPattNum].failDeviceMask[33]=2;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[0]=1;
        failPatterns[curPattNum].checkAnalogSignals[1]=1;
        failPatterns[curPattNum].checkAnalogSignals[2]=1;
        failPatterns[curPattNum].checkAnalogSignals[3]=1;
        failPatterns[curPattNum].checkAnalogSignals[4]=1;
        failPatterns[curPattNum].checkAnalogSignals[5]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;


        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  недостоверность P2
        curPattNum= 70 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x04;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = false;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);
        failPatterns[curPattNum].OUTB1_bit[1] = 0;

        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);


        failPatterns[curPattNum].failName = "Сигнал P2 недостоверен";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=2;
        failPatterns[curPattNum].failDeviceMask[34]=2;
        failPatterns[curPattNum].failDeviceMask[35]=2;
        failPatterns[curPattNum].failDeviceMask[36]=2;
        failPatterns[curPattNum].failDeviceMask[37]=2;
        failPatterns[curPattNum].failDeviceMask[38]=2;
        failPatterns[curPattNum].failDeviceMask[39]=2;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[6]=1;
        failPatterns[curPattNum].checkAnalogSignals[7]=1;
        failPatterns[curPattNum].checkAnalogSignals[8]=1;
        failPatterns[curPattNum].checkAnalogSignals[9]=1;
        failPatterns[curPattNum].checkAnalogSignals[10]=1;
        failPatterns[curPattNum].checkAnalogSignals[11]=1;


        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;


        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  недостоверность P1
        curPattNum= 71 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x04;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = false;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[3] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);
        failPatterns[curPattNum].OUTB1_bit[2] = 0;

        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);


        failPatterns[curPattNum].failName = "Сигнал P1 недостоверен. ";
        failPatterns[curPattNum].failSource = "АВВ14Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[1]=2;
        failPatterns[curPattNum].failDeviceMask[40]=2;
        failPatterns[curPattNum].failDeviceMask[41]=2;
        failPatterns[curPattNum].failDeviceMask[42]=2;

        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
        failPatterns[curPattNum].checkAnalogSignals[12]=1;
        failPatterns[curPattNum].checkAnalogSignals[13]=1;
        failPatterns[curPattNum].checkAnalogSignals[14]=1;



        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;



















    //------------------------------------------------diskret---------------------------------------------------------------
    //------------------------------------------------diskret---------------------------------------------------------------
    //------------------------------------------------diskret---------------------------------------------------------------
    //------------------------------------------------diskret---------------------------------------------------------------



        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  вывод в опробование комплекта АЗ
        curPattNum= 72 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x02;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = false;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[4] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);


        failPatterns[curPattNum].failName = "Вывод в опробование комплекта АЗ ";
        failPatterns[curPattNum].failSource = "1ЦВВ15М,2ЦВВ15М";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[2]=2;
        failPatterns[curPattNum].failDeviceMask[3]=2;


        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);




        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;



        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  Рассинхронизация работы шкафа по управляющему воздействию
        curPattNum= 73 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x02;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = false;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[5] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);


        failPatterns[curPattNum].failName = "Рассинхронизация работы шкафа по управляющему воздействию ";
        failPatterns[curPattNum].failSource = "3ЦВВ15М,УСК12";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[4]=2;
        failPatterns[curPattNum].failDeviceMask[5]=2;


        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);




        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;



        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  Рассинхронизация работы шкафа по управляющему воздействию
        curPattNum= 74 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x02;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = false;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[6] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);


        failPatterns[curPattNum].failName = "Рассинхронизация работы шкафа по дискретным сигналам ";
        failPatterns[curPattNum].failSource = "3ЦВВ15М,УСК12";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[4]=2;
        failPatterns[curPattNum].failDeviceMask[5]=2;


        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);




        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;


        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  Нет установки режима при включенном АРМ
        curPattNum= 75 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x02;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = false;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[7] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);


        failPatterns[curPattNum].failName = "Нет установки режима при включенном АРМ ";
        failPatterns[curPattNum].failSource = "ПМИ07Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[0]=2;



        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);




        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;



        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  Нет ответа ПМИ07Р
        curPattNum= 76 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x02;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
        failPatterns[curPattNum].isLocal = false;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);

        failPatterns[curPattNum].ValMK_bit[7] = 1;


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);


        failPatterns[curPattNum].failName = "Нет установки режима при включенном АРМ ";
        failPatterns[curPattNum].failSource = "ПМИ07Р";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[0]=2;



        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);




        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;


        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  Выключение автомата или неисправность системы питания
        curPattNum= 77 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x10;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog 16-valmkDIAG
        failPatterns[curPattNum].isLocal = false;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);


        memset(failPatterns[curPattNum].ValMKDiag_bit,5,8);
        failPatterns[curPattNum].ValMKDiag_bit[1] = 1; // диагностический канал!!!


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);


        failPatterns[curPattNum].failName = "Выключение автомата или неисправность системы питания ";
        failPatterns[curPattNum].failSource = "Система питания";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
        failPatterns[curPattNum].failDeviceMask[15]=2;
        failPatterns[curPattNum].failDeviceMask[16]=2;



        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);




        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;



        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  Открытые двери шкафа
        curPattNum= 78 ;
        //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
        failPatterns[curPattNum].checkBytes = 0x10;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog 16-valmkDIAG
        failPatterns[curPattNum].isLocal = false;

        memset(failPatterns[curPattNum].ValMK_bit,5,8);


        memset(failPatterns[curPattNum].ValMKDiag_bit,5,8);
        failPatterns[curPattNum].ValMKDiag_bit[2] = 1;// диагностический канал!!!


        memset(failPatterns[curPattNum].OUTB1_bit,5,8);


        memset(failPatterns[curPattNum].ValUSO_bit,5,8);

        memset(failPatterns[curPattNum].errSigValue,0,15);


        failPatterns[curPattNum].failName = "Открытые двери шкафа ";
        failPatterns[curPattNum].failSource = "Двери шкафа";

        //какие устройства зажигать
        memset(failPatterns[curPattNum].failDeviceMask,0,73);
       // failPatterns[curPattNum].failDeviceMask[0]=2;



        //какие аналоговые сигналы выводить
        memset(failPatterns[curPattNum].checkAnalogSignals,0,15);




        failPatterns[curPattNum].num = curPattNum;

        failPatterns[curPattNum].waitTimeOut = false;

        failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

        failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

        FailPattNum ++;




}
void FailStorage::initFailPatternsChan2(void)
{
    quint8 curPattNum=0;
    quint8 ustChanOffset=7;
    quint8 ustAnalogOffset=15;
   //------------------------------------------------Test ---------------------------------------------------------------

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  ОШибка файла с параметрами
       curPattNum=79;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x01;// 0x06  0-VALUSO 2-VALMK  4-OUTB3
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);



       memset(failPatterns[curPattNum].OUTB1_bit,5,8);

       memset(failPatterns[curPattNum].ValUSO_bit,5,8);
       failPatterns[curPattNum].ValUSO_bit[1] = 1;

       memset(failPatterns[curPattNum].errSig,0,15);


       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Ошибка файла с параметрами(Выдать ИСХ.)";
       failPatterns[curPattNum].failSource = "ПМИ07Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[0 + ustChanOffset]=2;


       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  Не прошел тест ПМИ07Р
       curPattNum=80;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x01;// 0x06  0-VALUSO 2-VALMK  4-OUTB3
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);



       memset(failPatterns[curPattNum].OUTB1_bit,5,8);

       memset(failPatterns[curPattNum].ValUSO_bit,5,8);
       failPatterns[curPattNum].ValUSO_bit[2] = 1;


       memset(failPatterns[curPattNum].errSig,0,15);


       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Не прошел тест процессора ПМИ07Р(Выдать ИСХ.)";
       failPatterns[curPattNum].failSource = "ПМИ07Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[0 + ustChanOffset]=2;


       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  Не прошел тест АВВ14Р
       curPattNum=81;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x01;// 0x06  0-VALUSO 2-VALMK  4-OUTB3
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);



       memset(failPatterns[curPattNum].OUTB1_bit,5,8);

       memset(failPatterns[curPattNum].ValUSO_bit,5,8);
       failPatterns[curPattNum].ValUSO_bit[3] = 1;

       memset(failPatterns[curPattNum].errSig,0,15);


       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Не прошел тест АВВ14Р(Выдать ИСХ.)";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=2;


       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  Не прошел тест 1ЦВВ15М
       curPattNum=82;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x01;// 0x06  0-VALUSO 2-VALMK  4-OUTB3
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);



       memset(failPatterns[curPattNum].OUTB1_bit,5,8);

       memset(failPatterns[curPattNum].ValUSO_bit,5,8);
       failPatterns[curPattNum].ValUSO_bit[4] = 1;

       memset(failPatterns[curPattNum].errSig,0,15);


       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Не прошел тест 1ЦВВ15М(Выдать ИСХ.)";
       failPatterns[curPattNum].failSource = "ЦВВ15М";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[2 + ustChanOffset]=2;


       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  Не прошел тест 2ЦВВ15М
       curPattNum=83;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x01;// 0x06  0-VALUSO 2-VALMK  4-OUTB3
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);



       memset(failPatterns[curPattNum].OUTB1_bit,5,8);

       memset(failPatterns[curPattNum].ValUSO_bit,5,8);
       failPatterns[curPattNum].ValUSO_bit[5] = 1;

       memset(failPatterns[curPattNum].errSig,0,15);


       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Не прошел тест 2ЦВВ15М(Выдать ИСХ.)";
       failPatterns[curPattNum].failSource = "ЦВВ15М";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[3 + ustChanOffset]=2;


       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  Не прошел тест 3ЦВВ15М
       curPattNum=84;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x01;// 0x06  0-VALUSO 2-VALMK  4-OUTB3
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);



       memset(failPatterns[curPattNum].OUTB1_bit,5,8);

       memset(failPatterns[curPattNum].ValUSO_bit,5,8);
       failPatterns[curPattNum].ValUSO_bit[6] = 1;

       memset(failPatterns[curPattNum].errSig,0,15);


       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Не прошел тест 3ЦВВ15М(Выдать ИСХ.)";
       failPatterns[curPattNum].failSource = "ЦВВ15М";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[4 + ustChanOffset]=2;


       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  Нет связи по RS485
       curPattNum=85;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x02;// 0x06  0-VALUSO 2-VALMK  4-OUTB3
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);
       failPatterns[curPattNum].ValMK_bit[0] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);

       memset(failPatterns[curPattNum].ValUSO_bit,5,8);


       memset(failPatterns[curPattNum].errSig,0,15);


       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Нет связи по R485(Выдать ИСХ.)";
       failPatterns[curPattNum].failSource = "ПМИ07Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[0 + ustChanOffset]=2;


       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  Останов канала
       curPattNum = 86;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB3 3-SGNCNTR

       failPatterns[curPattNum].checkBytes = 0x02;//  0-VALUSO 2-VALMK  4-OUTB3 8-ErrNo
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);
       failPatterns[curPattNum].ValMK_bit[1] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSig,0,15);


       failPatterns[curPattNum].failName = "Останов канала 2";
       failPatterns[curPattNum].failSource = "ПМИ07Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[0 + ustChanOffset]=1;



       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);



       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =1; // 1*100mls

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  нет приема сигнала
       curPattNum = 87;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB3 3-SGNCNTR

       failPatterns[curPattNum].checkBytes = 0x02;//  0-VALUSO 2-VALMK  4-OUTB3 8-ErrNo
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);
       failPatterns[curPattNum].ValMK_bit[2] = 1;
       failPatterns[curPattNum].ValMK_bit[3] = 1;

       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSig,0,15);


       failPatterns[curPattNum].failName = "Нет приема сигнала";
       failPatterns[curPattNum].failSource = "ПМИ07Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[0 + ustChanOffset]=1;



       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);



       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =1; // 1*100mls

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;









   //------------------------------------------------N---------------------------------------------------------------



       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность меньше границы (6 неисправностей)
       curPattNum= 88 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;

       memset(failPatterns[curPattNum].OUTB1_bit,5,8);

       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[0] = 0x02;


       failPatterns[curPattNum].failName = "Сигнал N.11 ниже допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[28 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[0]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность меньше границы (6 неисправностей)
       curPattNum= 89 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[1] = 0x02;


       failPatterns[curPattNum].failName = "Сигнал N.21 ниже допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[29 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[1]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность меньше границы (6 неисправностей)
       curPattNum= 90;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[2] = 0x02;


       failPatterns[curPattNum].failName = "Сигнал N.31 ниже допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[30 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[2]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность меньше границы (6 неисправностей)
       curPattNum= 91 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[3] = 0x02;


       failPatterns[curPattNum].failName = "Сигнал N.12 ниже допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[31 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[3]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность меньше границы (6 неисправностей)
       curPattNum= 92 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[4] = 0x02;


       failPatterns[curPattNum].failName = "Сигнал N.22 ниже допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[32 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[4]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность меньше границы (6 неисправностей)
       curPattNum= 93 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[5] = 0x02;


       failPatterns[curPattNum].failName = "Сигнал N.32 ниже допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[33 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[5]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;






       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность выше границы (6 неисправностей)
       curPattNum= 94 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[0] = 0x04;


       failPatterns[curPattNum].failName = "Сигнал N.11 выше допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[28  + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[0]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность выше границы (6 неисправностей)
       curPattNum= 95 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[1] = 0x04;


       failPatterns[curPattNum].failName = "Сигнал N.21 выше допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[29 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[1]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность выше границы (6 неисправностей)
       curPattNum= 96 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[2] = 0x04;


       failPatterns[curPattNum].failName = "Сигнал N.31 выше допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[30 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[2]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность выше границы (6 неисправностей)
       curPattNum= 97;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[3] = 0x04;


       failPatterns[curPattNum].failName = "Сигнал N.12 выше допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[31 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[3]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность выше границы (6 неисправностей)
       curPattNum= 98 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[4] = 0x04;




       failPatterns[curPattNum].failName = "Сигнал N.22 выше допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[32 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[4]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность выше границы (6 неисправностей)
       curPattNum= 99 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[5] = 0x04;


       failPatterns[curPattNum].failName = "Сигнал N.32 выше допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[33 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[5]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;




       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность обрыв линии (6 неисправностей)
       curPattNum= 100 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[0] = 0x01;

       failPatterns[curPattNum].failName = "Сигнал N.11, обрыв линии";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[28 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[0]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;


       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность обрыв линии (6 неисправностей)
       curPattNum= 101 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[1] = 0x01;

       failPatterns[curPattNum].failName = "Сигнал N.21, обрыв линии";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[29 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[1]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;


       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность обрыв линии (6 неисправностей)
       curPattNum= 102 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[2] = 0x01;

       failPatterns[curPattNum].failName = "Сигнал N.31, обрыв линии";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[30  + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[2]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность обрыв линии (6 неисправностей)
       curPattNum= 103 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[3] = 0x01;

       failPatterns[curPattNum].failName = "Сигнал N.12, обрыв линии";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[31 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[3]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность обрыв линии (6 неисправностей)
       curPattNum= 104 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[4] = 0x01;

       failPatterns[curPattNum].failName = "Сигнал N.22, обрыв линии";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[32 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[4]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность обрыв линии (6 неисправностей)
       curPattNum= 105 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[5] = 0x01;

       failPatterns[curPattNum].failName = "Сигнал N.32, обрыв линии";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[33 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[5]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;




       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность рассогласование (6 неисправностей)
       curPattNum= 106;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[0] = 0x08;

       failPatterns[curPattNum].failName = "Сигнал N.11, рассогласование сигналов";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[28 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[0]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность рассогласование (6 неисправностей)
       curPattNum= 107;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[1] = 0x08;

       failPatterns[curPattNum].failName = "Сигнал N.21, рассогласование сигналов";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[29 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[1]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность рассогласование (6 неисправностей)
       curPattNum= 108;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[2] = 0x08;

       failPatterns[curPattNum].failName = "Сигнал N.31, рассогласование сигналов";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[30 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[2]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность рассогласование (6 неисправностей)
       curPattNum= 109;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[3] = 0x08;

       failPatterns[curPattNum].failName = "Сигнал N.12, рассогласование сигналов";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1  + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[31 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[3]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность рассогласование (6 неисправностей)
       curPattNum= 110;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[4] = 0x08;

       failPatterns[curPattNum].failName = "Сигнал N.22, рассогласование сигналов";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[32 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[4]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность рассогласование (6 неисправностей)
       curPattNum= 111;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[5] = 0x08;

       failPatterns[curPattNum].failName = "Сигнал N.32, рассогласование сигналов";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[33 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[5]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
















   //------------------------------------------------P2---------------------------------------------------------------
   //------------------------------------------------P2---------------------------------------------------------------
   //------------------------------------------------P2---------------------------------------------------------------
   //------------------------------------------------P2---------------------------------------------------------------
   //------------------------------------------------P2---------------------------------------------------------------


       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК меньше границы (6 неисправностей)
       curPattNum= 112 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;

       memset(failPatterns[curPattNum].OUTB1_bit,5,8);

       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[6] = 0x02;


       failPatterns[curPattNum].failName = "Сигнал P2.1 ниже допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[34 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[6]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК меньше границы (6 неисправностей)
       curPattNum= 113 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[7] = 0x02;


       failPatterns[curPattNum].failName = "Сигнал P2.2 ниже допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[35 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[7]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК меньше границы (6 неисправностей)
       curPattNum= 114;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[8] = 0x02;


       failPatterns[curPattNum].failName = "Сигнал P2.3 ниже допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[36 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[8]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК меньше границы (6 неисправностей)
       curPattNum= 115 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[9] = 0x02;


       failPatterns[curPattNum].failName = "Сигнал P2.4 ниже допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[37 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[9]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК меньше границы (6 неисправностей)
       curPattNum= 116 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[10] = 0x02;


       failPatterns[curPattNum].failName = "Сигнал P2.5 ниже допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[38 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[10]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК меньше границы (6 неисправностей)
       curPattNum= 117 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[11] = 0x02;


       failPatterns[curPattNum].failName = "Сигнал P2.6 ниже допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[39 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[11]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;






       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК выше границы (6 неисправностей)
       curPattNum= 118 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[6] = 0x04;


       failPatterns[curPattNum].failName = "Сигнал P2.1 выше допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[34 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[6]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК выше границы (6 неисправностей)
       curPattNum= 119 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[7] = 0x04;


       failPatterns[curPattNum].failName = "Сигнал P2.2 выше допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[35 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[7]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК выше границы (6 неисправностей)
       curPattNum= 120;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[8] = 0x04;


       failPatterns[curPattNum].failName = "Сигнал P2.3 выше допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[36 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[8]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК выше границы (6 неисправностей)
       curPattNum= 121;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[9] = 0x04;


       failPatterns[curPattNum].failName = "Сигнал P2.4 выше допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[37 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[9]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК выше границы (6 неисправностей)
       curPattNum= 122 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[10] = 0x04;




       failPatterns[curPattNum].failName = "Сигнал P2.5 выше допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[38 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[10]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК выше границы (6 неисправностей)
       curPattNum= 123 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[11] = 0x04;


       failPatterns[curPattNum].failName = "Сигнал P2.6 выше допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[39 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[11]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;




       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК обрыв линии (6 неисправностей)
       curPattNum= 124 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[6] = 0x01;

       failPatterns[curPattNum].failName = "Сигнал P2.1, обрыв линии";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[34 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[6]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;


       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК обрыв линии (6 неисправностей)
       curPattNum= 125 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[7] = 0x01;

       failPatterns[curPattNum].failName = "Сигнал P2.2, обрыв линии";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[35 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[7]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;


       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК обрыв линии (6 неисправностей)
       curPattNum= 126 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[8] = 0x01;

       failPatterns[curPattNum].failName = "Сигнал P2.3, обрыв линии";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[36 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[8]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК обрыв линии (6 неисправностей)
       curPattNum= 127 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[9] = 0x01;

       failPatterns[curPattNum].failName = "Сигнал P2.4, обрыв линии";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[37 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[9]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК обрыв линии (6 неисправностей)
       curPattNum= 128 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[10] = 0x01;

       failPatterns[curPattNum].failName = "Сигнал P2.5, обрыв линии";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[38 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[10]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК обрыв линии (6 неисправностей)
       curPattNum= 129 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[11] = 0x01;

       failPatterns[curPattNum].failName = "Сигнал P2.6, обрыв линии";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[39 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[11]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;




       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК рассогласование (6 неисправностей)
       curPattNum= 130;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[6] = 0x08;

       failPatterns[curPattNum].failName = "Сигнал P2.1, рассогласование сигналов";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[34 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[6]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК рассогласование (6 неисправностей)
       curPattNum= 131;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[7] = 0x08;

       failPatterns[curPattNum].failName = "Сигнал P2.2, рассогласование сигналов";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[35 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[7]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК рассогласование (6 неисправностей)
       curPattNum= 132;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[8] = 0x08;

       failPatterns[curPattNum].failName = "Сигнал P2.3, рассогласование сигналов";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[36 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[8]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК рассогласование (6 неисправностей)
       curPattNum= 133;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[9] = 0x08;

       failPatterns[curPattNum].failName = "Сигнал P2.4, рассогласование сигналов";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[37 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[9]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК рассогласование (6 неисправностей)
       curPattNum= 134;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[10] = 0x08;

       failPatterns[curPattNum].failName = "Сигнал P2.5, рассогласование сигналов";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[38 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[10]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК рассогласование (6 неисправностей)
       curPattNum= 135;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[11] = 0x08;

       failPatterns[curPattNum].failName = "Сигнал P2.6, рассогласование сигналов";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[39 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[11]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;


















   //------------------------------------------------P1---------------------------------------------------------------
   //------------------------------------------------P1---------------------------------------------------------------
   //------------------------------------------------P1---------------------------------------------------------------
   //------------------------------------------------P1---------------------------------------------------------------

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление в первом контуре меньше границы (3 неисправностей)
       curPattNum= 136 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;

       memset(failPatterns[curPattNum].OUTB1_bit,5,8);

       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[12] = 0x02;


       failPatterns[curPattNum].failName = "Сигнал P1.1 ниже допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[40 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[12]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление в первом контуре меньше границы (3 неисправностей)
       curPattNum= 137 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[13] = 0x02;


       failPatterns[curPattNum].failName = "Сигнал P1.2 ниже допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[41 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[13]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление в первом контуре меньше границы (3 неисправностей)
       curPattNum= 138;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[14] = 0x02;


       failPatterns[curPattNum].failName = "Сигнал P1.3 ниже допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[42 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[14]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;







       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление в первом контуре выше границы (3 неисправностей)
       curPattNum= 139 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[12] = 0x04;


       failPatterns[curPattNum].failName = "Сигнал P1.1 выше допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[40 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[12]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление в первом контуре выше границы (3 неисправностей)
       curPattNum= 140 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[13] = 0x04;


       failPatterns[curPattNum].failName = "Сигнал P1.2 выше допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[41 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[13]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление в первом контуре выше границы (3 неисправностей)
       curPattNum= 141;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[14] = 0x04;


       failPatterns[curPattNum].failName = "Сигнал P1.3 выше допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[42  + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[14]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;




       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление в первом контуре обрыв линии (3 неисправностей)
       curPattNum= 142 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[12] = 0x01;

       failPatterns[curPattNum].failName = "Сигнал P1.1, обрыв линии";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[40 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[12]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;


       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление в первом контуре обрыв линии (3 неисправностей)
       curPattNum= 143 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[13] = 0x01;

       failPatterns[curPattNum].failName = "Сигнал P1.2, обрыв линии";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[41 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[13]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;


       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление в первом контуре обрыв линии (3 неисправностей)
       curPattNum= 144 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[14] = 0x01;

       failPatterns[curPattNum].failName = "Сигнал P1.3, обрыв линии";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[42 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[14]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;




       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление в первом контуре рассогласование (3 неисправностей)
       curPattNum= 145;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[12] = 0x08;

       failPatterns[curPattNum].failName = "Сигнал P1.1, рассогласование сигналов";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[40 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[12]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление в первом контуре рассогласование (3 неисправностей)
       curPattNum= 146;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[13] = 0x08;

       failPatterns[curPattNum].failName = "Сигнал P1.2, рассогласование сигналов";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[41 + ustChanOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[13]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление в первом контуре рассогласование (3 неисправностей)
       curPattNum= 147;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[14] = 0x08;

       failPatterns[curPattNum].failName = "Сигнал P1.3, рассогласование сигналов";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[42 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[14]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;




   //------------------------------------------------analoginvalid---------------------------------------------------------------
   //------------------------------------------------analoginvalid---------------------------------------------------------------
   //------------------------------------------------analoginvalid---------------------------------------------------------------
   //------------------------------------------------analoginvalid---------------------------------------------------------------
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  недостоверность N
       curPattNum= 148 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x04;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = false;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);
       failPatterns[curPattNum].OUTB1_bit[0] = 0;

       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Сигнал N недостоверен";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=2;
       failPatterns[curPattNum].failDeviceMask[28 + ustAnalogOffset]=2;
       failPatterns[curPattNum].failDeviceMask[29 + ustAnalogOffset]=2;
       failPatterns[curPattNum].failDeviceMask[30 + ustAnalogOffset]=2;
       failPatterns[curPattNum].failDeviceMask[31 + ustAnalogOffset]=2;
       failPatterns[curPattNum].failDeviceMask[32 + ustAnalogOffset]=2;
       failPatterns[curPattNum].failDeviceMask[33 + ustAnalogOffset]=2;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[0]=1;
       failPatterns[curPattNum].checkAnalogSignals[1]=1;
       failPatterns[curPattNum].checkAnalogSignals[2]=1;
       failPatterns[curPattNum].checkAnalogSignals[3]=1;
       failPatterns[curPattNum].checkAnalogSignals[4]=1;
       failPatterns[curPattNum].checkAnalogSignals[5]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;


       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  недостоверность P2
       curPattNum= 149 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x04;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = false;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);
       failPatterns[curPattNum].OUTB1_bit[1] = 0;

       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Сигнал P2 недостоверен";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=2;
       failPatterns[curPattNum].failDeviceMask[34 + ustAnalogOffset]=2;
       failPatterns[curPattNum].failDeviceMask[35 + ustAnalogOffset]=2;
       failPatterns[curPattNum].failDeviceMask[36 + ustAnalogOffset]=2;
       failPatterns[curPattNum].failDeviceMask[37 + ustAnalogOffset]=2;
       failPatterns[curPattNum].failDeviceMask[38 + ustAnalogOffset]=2;
       failPatterns[curPattNum].failDeviceMask[39 + ustAnalogOffset]=2;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[6]=1;
       failPatterns[curPattNum].checkAnalogSignals[7]=1;
       failPatterns[curPattNum].checkAnalogSignals[8]=1;
       failPatterns[curPattNum].checkAnalogSignals[9]=1;
       failPatterns[curPattNum].checkAnalogSignals[10]=1;
       failPatterns[curPattNum].checkAnalogSignals[11]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;


       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  недостоверность P1
       curPattNum= 150 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x04;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = false;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);
       failPatterns[curPattNum].OUTB1_bit[2] = 0;

       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Сигнал P1 недостоверен. ";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=2;
       failPatterns[curPattNum].failDeviceMask[40 + ustAnalogOffset]=2;
       failPatterns[curPattNum].failDeviceMask[41 + ustAnalogOffset]=2;
       failPatterns[curPattNum].failDeviceMask[42 + ustAnalogOffset]=2;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[12]=1;
       failPatterns[curPattNum].checkAnalogSignals[13]=1;
       failPatterns[curPattNum].checkAnalogSignals[14]=1;



       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;



















   //------------------------------------------------diskret---------------------------------------------------------------
   //------------------------------------------------diskret---------------------------------------------------------------
   //------------------------------------------------diskret---------------------------------------------------------------
   //------------------------------------------------diskret---------------------------------------------------------------



       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  вывод в опробование комплекта АЗ
       curPattNum= 151 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x02;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = false;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[4] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Вывод в опробование комплекта АЗ ";
       failPatterns[curPattNum].failSource = "1ЦВВ15М,2ЦВВ15М";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[2 + ustChanOffset]=2;
       failPatterns[curPattNum].failDeviceMask[3 + ustChanOffset]=2;


       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);




       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;



       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  Рассинхронизация работы шкафа по управляющему воздействию
       curPattNum= 152 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x02;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = false;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[5] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Рассинхронизация работы шкафа по управляющему воздействию ";
       failPatterns[curPattNum].failSource = "3ЦВВ15М,УСК12";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[4 + ustChanOffset]=2;
       failPatterns[curPattNum].failDeviceMask[5 + ustChanOffset]=2;


       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);




       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;



       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  Рассинхронизация работы шкафа по управляющему воздействию
       curPattNum= 153 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x02;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = false;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[6] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Рассинхронизация работы шкафа по дискретным сигналам ";
       failPatterns[curPattNum].failSource = "3ЦВВ15М,УСК12";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[4 + ustChanOffset]=2;
       failPatterns[curPattNum].failDeviceMask[5 + ustChanOffset]=2;


       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);




       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;


       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  Нет установки режима при включенном АРМ
       curPattNum= 154 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x02;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = false;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[7] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Нет установки режима при включенном АРМ ";
       failPatterns[curPattNum].failSource = "ПМИ07Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[0 + ustChanOffset]=2;



       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);




       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;



       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  Нет ответа ПМИ07Р
       curPattNum= 155 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x02;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = false;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[7] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Нет установки режима при включенном АРМ ";
       failPatterns[curPattNum].failSource = "ПМИ07Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[0 + ustChanOffset]=2;



       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);




       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;


       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  Выключение автомата или неисправность системы питания
       curPattNum= 156 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x10;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog 16-valmkDIAG
       failPatterns[curPattNum].isLocal = false;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);


       memset(failPatterns[curPattNum].ValMKDiag_bit,5,8);
       failPatterns[curPattNum].ValMKDiag_bit[1] = 1; // диагностический канал!!!


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Выключение автомата или неисправность системы питания ";
       failPatterns[curPattNum].failSource = "Система питания";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[15] =2;
       failPatterns[curPattNum].failDeviceMask[16] =2;//0 + ustChanOffset]=2;



       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);




       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;



       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  Открытые двери шкафа
       curPattNum= 157 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x10;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog 16-valmkDIAG
       failPatterns[curPattNum].isLocal = false;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);


       memset(failPatterns[curPattNum].ValMKDiag_bit,5,8);
       failPatterns[curPattNum].ValMKDiag_bit[2] = 1;// диагностический канал!!!


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Открытые двери шкафа ";
       failPatterns[curPattNum].failSource = "Двери шкафа";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       //failPatterns[curPattNum].failDeviceMask[0 + ustChanOffset]=2;



       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);




       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;



}
void FailStorage::initFailPatternsChan3(void)
{
    quint8 curPattNum=0;
    quint8 ustChanOffset=21;
    quint8 ustAnalogOffset=30;
    quint8 pattNumOffset=79;
   //------------------------------------------------Test ---------------------------------------------------------------

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  ОШибка файла с параметрами
       curPattNum =pattNumOffset + 79;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x01;// 0x06  0-VALUSO 2-VALMK  4-OUTB3
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);



       memset(failPatterns[curPattNum].OUTB1_bit,5,8);

       memset(failPatterns[curPattNum].ValUSO_bit,5,8);
       failPatterns[curPattNum].ValUSO_bit[1] = 1;

       memset(failPatterns[curPattNum].errSig,0,15);


       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Ошибка файла с параметрами(Выдать ИСХ.)";
       failPatterns[curPattNum].failSource = "ПМИ07Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[0 + ustChanOffset]=2;


       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  Не прошел тест ПМИ07Р
       curPattNum =pattNumOffset + 80;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x01;// 0x06  0-VALUSO 2-VALMK  4-OUTB3
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);



       memset(failPatterns[curPattNum].OUTB1_bit,5,8);

       memset(failPatterns[curPattNum].ValUSO_bit,5,8);
       failPatterns[curPattNum].ValUSO_bit[2] = 1;


       memset(failPatterns[curPattNum].errSig,0,15);


       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Не прошел тест процессора ПМИ07Р(Выдать ИСХ.)";
       failPatterns[curPattNum].failSource = "ПМИ07Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[0 + ustChanOffset]=2;


       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  Не прошел тест АВВ14Р
       curPattNum =pattNumOffset + 81;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x01;// 0x06  0-VALUSO 2-VALMK  4-OUTB3
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);



       memset(failPatterns[curPattNum].OUTB1_bit,5,8);

       memset(failPatterns[curPattNum].ValUSO_bit,5,8);
       failPatterns[curPattNum].ValUSO_bit[3] = 1;

       memset(failPatterns[curPattNum].errSig,0,15);


       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Не прошел тест АВВ14Р(Выдать ИСХ.)";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=2;


       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  Не прошел тест 1ЦВВ15М
       curPattNum =pattNumOffset + 82;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x01;// 0x06  0-VALUSO 2-VALMK  4-OUTB3
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);



       memset(failPatterns[curPattNum].OUTB1_bit,5,8);

       memset(failPatterns[curPattNum].ValUSO_bit,5,8);
       failPatterns[curPattNum].ValUSO_bit[4] = 1;

       memset(failPatterns[curPattNum].errSig,0,15);


       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Не прошел тест 1ЦВВ15М(Выдать ИСХ.)";
       failPatterns[curPattNum].failSource = "ЦВВ15М";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[2 + ustChanOffset]=2;


       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  Не прошел тест 2ЦВВ15М
       curPattNum =pattNumOffset + 83;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x01;// 0x06  0-VALUSO 2-VALMK  4-OUTB3
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);



       memset(failPatterns[curPattNum].OUTB1_bit,5,8);

       memset(failPatterns[curPattNum].ValUSO_bit,5,8);
       failPatterns[curPattNum].ValUSO_bit[5] = 1;

       memset(failPatterns[curPattNum].errSig,0,15);


       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Не прошел тест 2ЦВВ15М(Выдать ИСХ.)";
       failPatterns[curPattNum].failSource = "ЦВВ15М";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[3 + ustChanOffset]=2;


       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  Не прошел тест 3ЦВВ15М
       curPattNum =pattNumOffset + 84;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x01;// 0x06  0-VALUSO 2-VALMK  4-OUTB3
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);



       memset(failPatterns[curPattNum].OUTB1_bit,5,8);

       memset(failPatterns[curPattNum].ValUSO_bit,5,8);
       failPatterns[curPattNum].ValUSO_bit[6] = 1;

       memset(failPatterns[curPattNum].errSig,0,15);


       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Не прошел тест 3ЦВВ15М(Выдать ИСХ.)";
       failPatterns[curPattNum].failSource = "ЦВВ15М";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[4 + ustChanOffset]=2;


       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  Нет связи по RS485
       curPattNum =pattNumOffset + 85;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x02;// 0x06  0-VALUSO 2-VALMK  4-OUTB3
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);
       failPatterns[curPattNum].ValMK_bit[0] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);

       memset(failPatterns[curPattNum].ValUSO_bit,5,8);


       memset(failPatterns[curPattNum].errSig,0,15);


       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Нет связи по R485(Выдать ИСХ.)";
       failPatterns[curPattNum].failSource = "ПМИ07Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[0 + ustChanOffset]=2;


       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  Останов канала
       curPattNum =pattNumOffset + 86;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB3 3-SGNCNTR

       failPatterns[curPattNum].checkBytes = 0x02;//  0-VALUSO 2-VALMK  4-OUTB3 8-ErrNo
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);
       failPatterns[curPattNum].ValMK_bit[1] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSig,0,15);


       failPatterns[curPattNum].failName = "Останов канала 3";
       failPatterns[curPattNum].failSource = "ПМИ07Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[0 + ustChanOffset]=1;



       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);



       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =1; // 1*100mls

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  нет приема сигнала
       curPattNum =pattNumOffset + 87;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB3 3-SGNCNTR

       failPatterns[curPattNum].checkBytes = 0x02;//  0-VALUSO 2-VALMK  4-OUTB3 8-ErrNo
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);
       failPatterns[curPattNum].ValMK_bit[2] = 1;
       failPatterns[curPattNum].ValMK_bit[3] = 1;

       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSig,0,15);


       failPatterns[curPattNum].failName = "Нет приема сигнала";
       failPatterns[curPattNum].failSource = "ПМИ07Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[0 + ustChanOffset]=1;



       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);



       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =1; // 1*100mls

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;









   //------------------------------------------------N---------------------------------------------------------------



       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность меньше границы (6 неисправностей)
       curPattNum =pattNumOffset +  88 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;

       memset(failPatterns[curPattNum].OUTB1_bit,5,8);

       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[0] = 0x02;


       failPatterns[curPattNum].failName = "Сигнал N.11 ниже допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[28 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[0]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность меньше границы (6 неисправностей)
       curPattNum =pattNumOffset +  89 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[1] = 0x02;


       failPatterns[curPattNum].failName = "Сигнал N.21 ниже допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[29 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[1]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность меньше границы (6 неисправностей)
       curPattNum =pattNumOffset +  90;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[2] = 0x02;


       failPatterns[curPattNum].failName = "Сигнал N.31 ниже допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[30 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[2]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность меньше границы (6 неисправностей)
       curPattNum =pattNumOffset +  91 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[3] = 0x02;


       failPatterns[curPattNum].failName = "Сигнал N.12 ниже допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[31 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[3]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность меньше границы (6 неисправностей)
       curPattNum =pattNumOffset +  92 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[4] = 0x02;


       failPatterns[curPattNum].failName = "Сигнал N.22 ниже допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[32 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[4]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность меньше границы (6 неисправностей)
       curPattNum =pattNumOffset +  93 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[5] = 0x02;


       failPatterns[curPattNum].failName = "Сигнал N.32 ниже допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[33 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[5]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;






       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность выше границы (6 неисправностей)
       curPattNum =pattNumOffset +  94 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[0] = 0x04;


       failPatterns[curPattNum].failName = "Сигнал N.11 выше допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[28  + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[0]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность выше границы (6 неисправностей)
       curPattNum =pattNumOffset +  95 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[1] = 0x04;


       failPatterns[curPattNum].failName = "Сигнал N.21 выше допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[29 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[1]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность выше границы (6 неисправностей)
       curPattNum =pattNumOffset +  96 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[2] = 0x04;


       failPatterns[curPattNum].failName = "Сигнал N.31 выше допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[30 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[2]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность выше границы (6 неисправностей)
       curPattNum =pattNumOffset +  97;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[3] = 0x04;


       failPatterns[curPattNum].failName = "Сигнал N.12 выше допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[31 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[3]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность выше границы (6 неисправностей)
       curPattNum =pattNumOffset +  98 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[4] = 0x04;




       failPatterns[curPattNum].failName = "Сигнал N.22 выше допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[32 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[4]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность выше границы (6 неисправностей)
       curPattNum =pattNumOffset +  99 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[5] = 0x04;


       failPatterns[curPattNum].failName = "Сигнал N.32 выше допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[33 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[5]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;




       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность обрыв линии (6 неисправностей)
       curPattNum =pattNumOffset +  100 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[0] = 0x01;

       failPatterns[curPattNum].failName = "Сигнал N.11, обрыв линии";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[28 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[0]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;


       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность обрыв линии (6 неисправностей)
       curPattNum =pattNumOffset +  101 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[1] = 0x01;

       failPatterns[curPattNum].failName = "Сигнал N.21, обрыв линии";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[29 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[1]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;


       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность обрыв линии (6 неисправностей)
       curPattNum =pattNumOffset +  102 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[2] = 0x01;

       failPatterns[curPattNum].failName = "Сигнал N.31, обрыв линии";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[30  + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[2]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность обрыв линии (6 неисправностей)
       curPattNum =pattNumOffset +  103 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[3] = 0x01;

       failPatterns[curPattNum].failName = "Сигнал N.12, обрыв линии";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[31 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[3]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность обрыв линии (6 неисправностей)
       curPattNum =pattNumOffset +  104 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[4] = 0x01;

       failPatterns[curPattNum].failName = "Сигнал N.22, обрыв линии";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[32 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[4]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность обрыв линии (6 неисправностей)
       curPattNum =pattNumOffset +  105 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[5] = 0x01;

       failPatterns[curPattNum].failName = "Сигнал N.32, обрыв линии";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[33 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[5]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;




       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность рассогласование (6 неисправностей)
       curPattNum =pattNumOffset +  106;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[0] = 0x08;

       failPatterns[curPattNum].failName = "Сигнал N.11, рассогласование сигналов";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[28 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[0]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность рассогласование (6 неисправностей)
       curPattNum =pattNumOffset +  107;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[1] = 0x08;

       failPatterns[curPattNum].failName = "Сигнал N.21, рассогласование сигналов";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[29 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[1]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность рассогласование (6 неисправностей)
       curPattNum =pattNumOffset +  108;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[2] = 0x08;

       failPatterns[curPattNum].failName = "Сигнал N.31, рассогласование сигналов";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[30 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[2]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность рассогласование (6 неисправностей)
       curPattNum =pattNumOffset +  109;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[3] = 0x08;

       failPatterns[curPattNum].failName = "Сигнал N.12, рассогласование сигналов";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1  + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[31 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[3]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность рассогласование (6 неисправностей)
       curPattNum =pattNumOffset +  110;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[4] = 0x08;

       failPatterns[curPattNum].failName = "Сигнал N.22, рассогласование сигналов";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[32 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[4]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая нейтронная мощность рассогласование (6 неисправностей)
       curPattNum =pattNumOffset +  111;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[5] = 0x08;

       failPatterns[curPattNum].failName = "Сигнал N.32, рассогласование сигналов";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[33 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[5]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
















   //------------------------------------------------P2---------------------------------------------------------------
   //------------------------------------------------P2---------------------------------------------------------------
   //------------------------------------------------P2---------------------------------------------------------------
   //------------------------------------------------P2---------------------------------------------------------------
   //------------------------------------------------P2---------------------------------------------------------------


       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК меньше границы (6 неисправностей)
       curPattNum =pattNumOffset +  112 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;

       memset(failPatterns[curPattNum].OUTB1_bit,5,8);

       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[6] = 0x02;


       failPatterns[curPattNum].failName = "Сигнал P2.1 ниже допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[34 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[6]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК меньше границы (6 неисправностей)
       curPattNum =pattNumOffset +  113 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[7] = 0x02;


       failPatterns[curPattNum].failName = "Сигнал P2.2 ниже допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[35 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[7]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК меньше границы (6 неисправностей)
       curPattNum =pattNumOffset +  114;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[8] = 0x02;


       failPatterns[curPattNum].failName = "Сигнал P2.3 ниже допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[36 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[8]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК меньше границы (6 неисправностей)
       curPattNum =pattNumOffset +  115 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[9] = 0x02;


       failPatterns[curPattNum].failName = "Сигнал P2.4 ниже допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[37 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[9]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК меньше границы (6 неисправностей)
       curPattNum =pattNumOffset +  116 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[10] = 0x02;


       failPatterns[curPattNum].failName = "Сигнал P2.5 ниже допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[38 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[10]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК меньше границы (6 неисправностей)
       curPattNum =pattNumOffset +  117 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[11] = 0x02;


       failPatterns[curPattNum].failName = "Сигнал P2.6 ниже допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[39 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[11]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;






       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК выше границы (6 неисправностей)
       curPattNum =pattNumOffset +  118 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[6] = 0x04;


       failPatterns[curPattNum].failName = "Сигнал P2.1 выше допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[34 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[6]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК выше границы (6 неисправностей)
       curPattNum =pattNumOffset +  119 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[7] = 0x04;


       failPatterns[curPattNum].failName = "Сигнал P2.2 выше допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[35 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[7]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК выше границы (6 неисправностей)
       curPattNum =pattNumOffset +  120;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[8] = 0x04;


       failPatterns[curPattNum].failName = "Сигнал P2.3 выше допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[36 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[8]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК выше границы (6 неисправностей)
       curPattNum =pattNumOffset +  121;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[9] = 0x04;


       failPatterns[curPattNum].failName = "Сигнал P2.4 выше допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[37 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[9]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК выше границы (6 неисправностей)
       curPattNum =pattNumOffset +  122 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[10] = 0x04;




       failPatterns[curPattNum].failName = "Сигнал P2.5 выше допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[38 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[10]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК выше границы (6 неисправностей)
       curPattNum =pattNumOffset +  123 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[11] = 0x04;


       failPatterns[curPattNum].failName = "Сигнал P2.6 выше допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[39 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[11]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;




       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК обрыв линии (6 неисправностей)
       curPattNum =pattNumOffset +  124 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[6] = 0x01;

       failPatterns[curPattNum].failName = "Сигнал P2.1, обрыв линии";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[34 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[6]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;


       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК обрыв линии (6 неисправностей)
       curPattNum =pattNumOffset +  125 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[7] = 0x01;

       failPatterns[curPattNum].failName = "Сигнал P2.2, обрыв линии";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[35 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[7]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;


       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК обрыв линии (6 неисправностей)
       curPattNum =pattNumOffset +  126 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[8] = 0x01;

       failPatterns[curPattNum].failName = "Сигнал P2.3, обрыв линии";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[36 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[8]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК обрыв линии (6 неисправностей)
       curPattNum =pattNumOffset +  127 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[9] = 0x01;

       failPatterns[curPattNum].failName = "Сигнал P2.4, обрыв линии";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[37 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[9]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК обрыв линии (6 неисправностей)
       curPattNum =pattNumOffset +  128 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[10] = 0x01;

       failPatterns[curPattNum].failName = "Сигнал P2.5, обрыв линии";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[38 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[10]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК обрыв линии (6 неисправностей)
       curPattNum =pattNumOffset +  129 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[11] = 0x01;

       failPatterns[curPattNum].failName = "Сигнал P2.6, обрыв линии";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[39 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[11]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;




       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК рассогласование (6 неисправностей)
       curPattNum =pattNumOffset +  130;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[6] = 0x08;

       failPatterns[curPattNum].failName = "Сигнал P2.1, рассогласование сигналов";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[34 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[6]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК рассогласование (6 неисправностей)
       curPattNum =pattNumOffset +  131;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[7] = 0x08;

       failPatterns[curPattNum].failName = "Сигнал P2.2, рассогласование сигналов";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[35 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[7]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК рассогласование (6 неисправностей)
       curPattNum =pattNumOffset +  132;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[8] = 0x08;

       failPatterns[curPattNum].failName = "Сигнал P2.3, рассогласование сигналов";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[36 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[8]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК рассогласование (6 неисправностей)
       curPattNum =pattNumOffset +  133;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[9] = 0x08;

       failPatterns[curPattNum].failName = "Сигнал P2.4, рассогласование сигналов";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[37 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[9]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК рассогласование (6 неисправностей)
       curPattNum =pattNumOffset +  134;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[10] = 0x08;

       failPatterns[curPattNum].failName = "Сигнал P2.5, рассогласование сигналов";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[38 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[10]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление ГПК рассогласование (6 неисправностей)
       curPattNum =pattNumOffset +  135;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[11] = 0x08;

       failPatterns[curPattNum].failName = "Сигнал P2.6, рассогласование сигналов";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[39 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[11]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;


















   //------------------------------------------------P1---------------------------------------------------------------
   //------------------------------------------------P1---------------------------------------------------------------
   //------------------------------------------------P1---------------------------------------------------------------
   //------------------------------------------------P1---------------------------------------------------------------

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление в первом контуре меньше границы (3 неисправностей)
       curPattNum =pattNumOffset +  136 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;

       memset(failPatterns[curPattNum].OUTB1_bit,5,8);

       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[12] = 0x02;


       failPatterns[curPattNum].failName = "Сигнал P1.1 ниже допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[40 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[12]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление в первом контуре меньше границы (3 неисправностей)
       curPattNum =pattNumOffset +  137 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[13] = 0x02;


       failPatterns[curPattNum].failName = "Сигнал P1.2 ниже допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[41 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[13]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление в первом контуре меньше границы (3 неисправностей)
       curPattNum =pattNumOffset +  138;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[14] = 0x02;


       failPatterns[curPattNum].failName = "Сигнал P1.3 ниже допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[42 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[14]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;







       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление в первом контуре выше границы (3 неисправностей)
       curPattNum =pattNumOffset +  139 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[12] = 0x04;


       failPatterns[curPattNum].failName = "Сигнал P1.1 выше допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[40 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[12]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление в первом контуре выше границы (3 неисправностей)
       curPattNum =pattNumOffset +  140 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[13] = 0x04;


       failPatterns[curPattNum].failName = "Сигнал P1.2 выше допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[41 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[13]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление в первом контуре выше границы (3 неисправностей)
       curPattNum =pattNumOffset +  141;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[14] = 0x04;


       failPatterns[curPattNum].failName = "Сигнал P1.3 выше допустимой границы";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[42  + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[14]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;




       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление в первом контуре обрыв линии (3 неисправностей)
       curPattNum =pattNumOffset +  142 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[12] = 0x01;

       failPatterns[curPattNum].failName = "Сигнал P1.1, обрыв линии";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[40 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[12]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;


       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление в первом контуре обрыв линии (3 неисправностей)
       curPattNum =pattNumOffset +  143 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[13] = 0x01;

       failPatterns[curPattNum].failName = "Сигнал P1.2, обрыв линии";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[41 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[13]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;


       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление в первом контуре обрыв линии (3 неисправностей)
       curPattNum =pattNumOffset +  144 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[14] = 0x01;

       failPatterns[curPattNum].failName = "Сигнал P1.3, обрыв линии";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[42 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[14]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;




       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление в первом контуре рассогласование (3 неисправностей)
       curPattNum =pattNumOffset +  145;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[12] = 0x08;

       failPatterns[curPattNum].failName = "Сигнал P1.1, рассогласование сигналов";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[40 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[12]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление в первом контуре рассогласование (3 неисправностей)
       curPattNum =pattNumOffset +  146;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[13] = 0x08;

       failPatterns[curPattNum].failName = "Сигнал P1.2, рассогласование сигналов";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[41 + ustChanOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[13]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;

       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  каждая давление в первом контуре рассогласование (3 неисправностей)
       curPattNum =pattNumOffset +  147;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x0A;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = true;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);
       failPatterns[curPattNum].errSigValue[14] = 0x08;

       failPatterns[curPattNum].failName = "Сигнал P1.3, рассогласование сигналов";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=1;
       failPatterns[curPattNum].failDeviceMask[42 + ustAnalogOffset]=1;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[14]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;




   //------------------------------------------------analoginvalid---------------------------------------------------------------
   //------------------------------------------------analoginvalid---------------------------------------------------------------
   //------------------------------------------------analoginvalid---------------------------------------------------------------
   //------------------------------------------------analoginvalid---------------------------------------------------------------
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  недостоверность N
       curPattNum =pattNumOffset +  148 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x04;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = false;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);
       failPatterns[curPattNum].OUTB1_bit[0] = 0;

       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Сигнал N недостоверен";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=2;
       failPatterns[curPattNum].failDeviceMask[28 + ustAnalogOffset]=2;
       failPatterns[curPattNum].failDeviceMask[29 + ustAnalogOffset]=2;
       failPatterns[curPattNum].failDeviceMask[30 + ustAnalogOffset]=2;
       failPatterns[curPattNum].failDeviceMask[31 + ustAnalogOffset]=2;
       failPatterns[curPattNum].failDeviceMask[32 + ustAnalogOffset]=2;
       failPatterns[curPattNum].failDeviceMask[33 + ustAnalogOffset]=2;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[0]=1;
       failPatterns[curPattNum].checkAnalogSignals[1]=1;
       failPatterns[curPattNum].checkAnalogSignals[2]=1;
       failPatterns[curPattNum].checkAnalogSignals[3]=1;
       failPatterns[curPattNum].checkAnalogSignals[4]=1;
       failPatterns[curPattNum].checkAnalogSignals[5]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;


       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  недостоверность P2
       curPattNum =pattNumOffset +  149 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x04;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = false;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);
       failPatterns[curPattNum].OUTB1_bit[1] = 0;

       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Сигнал P2 недостоверен";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=2;
       failPatterns[curPattNum].failDeviceMask[34 + ustAnalogOffset]=2;
       failPatterns[curPattNum].failDeviceMask[35 + ustAnalogOffset]=2;
       failPatterns[curPattNum].failDeviceMask[36 + ustAnalogOffset]=2;
       failPatterns[curPattNum].failDeviceMask[37 + ustAnalogOffset]=2;
       failPatterns[curPattNum].failDeviceMask[38 + ustAnalogOffset]=2;
       failPatterns[curPattNum].failDeviceMask[39 + ustAnalogOffset]=2;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[6]=1;
       failPatterns[curPattNum].checkAnalogSignals[7]=1;
       failPatterns[curPattNum].checkAnalogSignals[8]=1;
       failPatterns[curPattNum].checkAnalogSignals[9]=1;
       failPatterns[curPattNum].checkAnalogSignals[10]=1;
       failPatterns[curPattNum].checkAnalogSignals[11]=1;


       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;


       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  недостоверность P1
       curPattNum =pattNumOffset +  150 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x04;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = false;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[3] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);
       failPatterns[curPattNum].OUTB1_bit[2] = 0;

       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Сигнал P1 недостоверен. ";
       failPatterns[curPattNum].failSource = "АВВ14Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[1 + ustChanOffset]=2;
       failPatterns[curPattNum].failDeviceMask[40 + ustAnalogOffset]=2;
       failPatterns[curPattNum].failDeviceMask[41 + ustAnalogOffset]=2;
       failPatterns[curPattNum].failDeviceMask[42 + ustAnalogOffset]=2;

       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);
       failPatterns[curPattNum].checkAnalogSignals[12]=1;
       failPatterns[curPattNum].checkAnalogSignals[13]=1;
       failPatterns[curPattNum].checkAnalogSignals[14]=1;



       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;



















   //------------------------------------------------diskret---------------------------------------------------------------
   //------------------------------------------------diskret---------------------------------------------------------------
   //------------------------------------------------diskret---------------------------------------------------------------
   //------------------------------------------------diskret---------------------------------------------------------------



       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  вывод в опробование комплекта АЗ
       curPattNum =pattNumOffset +  151 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x02;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = false;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[4] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Вывод в опробование комплекта АЗ ";
       failPatterns[curPattNum].failSource = "1ЦВВ15М,2ЦВВ15М";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[2 + ustChanOffset]=2;
       failPatterns[curPattNum].failDeviceMask[3 + ustChanOffset]=2;


       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);




       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;



       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  Рассинхронизация работы шкафа по управляющему воздействию
       curPattNum =pattNumOffset +  152 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x02;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = false;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[5] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Рассинхронизация работы шкафа по управляющему воздействию ";
       failPatterns[curPattNum].failSource = "3ЦВВ15М,УСК12";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[4 + ustChanOffset]=2;
       failPatterns[curPattNum].failDeviceMask[5 + ustChanOffset]=2;


       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);




       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;



       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  Рассинхронизация работы шкафа по управляющему воздействию
       curPattNum =pattNumOffset +  153 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x02;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = false;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[6] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Рассинхронизация работы шкафа по дискретным сигналам ";
       failPatterns[curPattNum].failSource = "3ЦВВ15М,УСК12";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[4 + ustChanOffset]=2;
       failPatterns[curPattNum].failDeviceMask[5 + ustChanOffset]=2;


       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);




       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;


       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  Нет установки режима при включенном АРМ
       curPattNum =pattNumOffset +  154 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x02;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = false;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[7] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Нет установки режима при включенном АРМ ";
       failPatterns[curPattNum].failSource = "ПМИ07Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[0 + ustChanOffset]=2;



       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);




       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;



       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  Нет ответа ПМИ07Р
       curPattNum =pattNumOffset +  155 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x02;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog
       failPatterns[curPattNum].isLocal = false;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);

       failPatterns[curPattNum].ValMK_bit[7] = 1;


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Нет установки режима при включенном АРМ ";
       failPatterns[curPattNum].failSource = "ПМИ07Р";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[0 + ustChanOffset]=2;



       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);




       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;


       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  Выключение автомата или неисправность системы питания
       curPattNum =pattNumOffset +  156 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x10;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog 16-valmkDIAG
       failPatterns[curPattNum].isLocal = false;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);


       memset(failPatterns[curPattNum].ValMKDiag_bit,5,8);
       failPatterns[curPattNum].ValMKDiag_bit[1] = 1; // диагностический канал!!!


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Выключение автомата или неисправность системы питания 3";
       failPatterns[curPattNum].failSource = "Система питания";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
       failPatterns[curPattNum].failDeviceMask[15] = 2;
       failPatterns[curPattNum].failDeviceMask[16] = 2;//0 + ustChanOffset]=2;



       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);




       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;



       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       //  Открытые двери шкафа
       curPattNum =pattNumOffset +  157 ;
       //какие байты проверяються 0-VALUSO 1-VALMK  2-OUTB1 3-SGNCNTR
       failPatterns[curPattNum].checkBytes = 0x10;// 0x06  0-VALUSO 2-VALMK  4-OUTB3 8-analog 16-valmkDIAG
       failPatterns[curPattNum].isLocal = false;

       memset(failPatterns[curPattNum].ValMK_bit,5,8);


       memset(failPatterns[curPattNum].ValMKDiag_bit,5,8);
       failPatterns[curPattNum].ValMKDiag_bit[2] = 1;// диагностический канал!!!


       memset(failPatterns[curPattNum].OUTB1_bit,5,8);


       memset(failPatterns[curPattNum].ValUSO_bit,5,8);

       memset(failPatterns[curPattNum].errSigValue,0,15);


       failPatterns[curPattNum].failName = "Открытые двери шкафа ";
       failPatterns[curPattNum].failSource = "Двери шкафа";

       //какие устройства зажигать
       memset(failPatterns[curPattNum].failDeviceMask,0,73);
      // failPatterns[curPattNum].failDeviceMask[0 + ustChanOffset]=2;



       //какие аналоговые сигналы выводить
       memset(failPatterns[curPattNum].checkAnalogSignals,0,15);




       failPatterns[curPattNum].num = curPattNum;

       failPatterns[curPattNum].waitTimeOut = false;

       failPatterns[curPattNum].failTimeConst =20; // 20*100mls = 2 sec

       failTimeOutMas[curPattNum] = 20; // 20*100mls = 2 sec

       FailPattNum ++;


}
void FailStorage::initFailPatternsChan4(void)
{

}

void FailStorage::initFailPatterns(void)
{

    FailPattNum = 0;


    initFailPatternsChan1();
    initFailPatternsChan2();
    initFailPatternsChan3();
    initFailPatternsChan4();








}

