#include "showdata.h"
#include "recordblackbox.h"


ShowData::ShowData(QObject *parent) : QObject(parent)
{

    pWork1= false;
    pWork2= false;
    pWork3= false;
    pWork4= false;

}


ShowData::~ShowData()
{

}

//void ShowData::on_new_txt1(QString *data)
//{
//    QString *buf=new  QString("\005\013\177");
//    QString *workString=new QString;
//    qint16 first=-1;
//    quint16 ln=0;
//    qint16 calcKS=0;
//    qint16 readKS=0;
//    qint16 i,j,k;
//    float flo;
//    int bt=0;

//#ifdef DEBUGSHOWDATA
//    qDebug("   ShowData newtxt1 start");
//#endif

//    RecordBlackBox *elem=new RecordBlackBox();


//    if ((data->length() + strcom1.length())>3500 )
//    {
//        strcom1.clear();
//    }
//    strcom1.append(*data);

//    if (!pWork1)
//        {pWork1=true;

//        first=strcom1.indexOf(*buf);
//        if (first>-1)
//            {

//                 if ((strcom1.length()-first)<=strcom1.length())
//                   strcom1=strcom1.right( strcom1.length()-first);

//                 if (strcom1.length()>=11)
//                 {
//                      ln =Trans4in2( strcom1.mid(7,4).data());
//                      readKS =Trans4in2( strcom1.mid(3,4).data());
//                      if (strcom1.length()>=(11+ln))
//                          {
//                           *workString = strcom1.mid(11,ln);
//                           strcom1=strcom1.right(strcom1.length()-ln);
//                           calcKS =KS(workString->data(),ln);
//                           if (calcKS==readKS)
//                           {
//                            i=0;
//                            k=0;
//                         for(j=0;j<51;j++)
//                         {
//                            k=i;
//                            i=workString->indexOf(" ",k+1);



//                            if (j<20) // Analogovie signaly
//                            {
//                                 flo =workString->mid(k,i-k).toFloat();
//                                 if (j<8)
//                                 {
//                                     elem->Anlg[j]=flo;
//                                 }else
//                                 {
//                                     elem->SgnAnl[j-8]=flo;
//                                 }
//                            }



//                            if (j>=20 && j<=23) // Vihodnie signaly OutCww
//                            {
//                               bt =workString->mid(k,i-k).toInt();
//                               if (j<22)
//                               {
//                                elem->OutCww1[j-20]=bt;
//                               }else
//                               {
//                                elem->OutCww3[j-22]=bt;
//                               }
//                            }



//                            if (j>=24 && j<=35) // Vhodnie signaly InCww
//                            {
//                               bt =workString->mid(k,i-k).toInt();
//                               if (j<30)
//                               {
//                                elem->InCww1[j-24]=bt;
//                               }else
//                               { if(j<34)
//                                 {
//                                   elem->InCww2[j-28]=bt;
//                                 }else
//                                 {
//                                    elem->InCww3[j-32]=bt;

//                                 }
//                               }
//                            }



//                            if (j>=36 && j<=47) // Dostovernost' 12ti analogovih signalov
//                            {
//                               bt =workString->mid(k,i-k).toInt();
//                                elem->dsAn[j-36]=bt;
//                            }


//                            if (j==48) // ValUSO
//                            {
//                               bt =workString->mid(k,i-k).toInt();
//                                elem->ValUSO = bt;
//                            }

//                            if (j==49) // ValMK
//                            {
//                               bt =workString->mid(k,i-k).toInt();
//                                elem->ValMK=bt;
//                            }



//                            if (j==50) // Vremya
//                            {int x=0;
//                                for(x=0;x<12;x++)
//                                    elem->time[x]=workString->mid(k+x+1,1).at(0).unicode();
//                                emit NewRecReadyMK1(*elem);
//                            }


//                           }//for i=0 to 50
//                          }
//                 }

//            }

//        }

//         pWork1=false;
//        }

//#ifdef DEBUGSHOWDATA
//    qDebug("   ShowData newtxt1 end");
//#endif
//}

void ShowData::on_new_txt1(QString *data)
{
    QString *buf=new  QString("\005\013\177");
    QString *workString=new QString;
    qint16 first=-1;
    quint16 ln=0;
    qint16 calcKS=0;
    qint16 readKS=0;
    qint16 i,j,k;
    float flo;
    int bt=0;

#ifdef DEBUGSHOWDATA
    qDebug("   ShowData newtxt1 start");
#endif

    RecordBlackBox *elem=new RecordBlackBox();


    if ((data->length() + strcom1.length())>3500 )
    {
        strcom1.clear();
    }
    strcom1.append(*data);

    if (!pWork1)
        {pWork1=true;

        first=strcom1.indexOf(*buf);
        if (first>-1)
            {

                 if ((strcom1.length()-first)<=strcom1.length())
                   strcom1=strcom1.right( strcom1.length()-first);

                 if (strcom1.length()>=11)
                 {
                      ln =Trans4in2( strcom1.mid(7,4).data());
                      readKS =Trans4in2( strcom1.mid(3,4).data());
                      if (strcom1.length()>=(11+ln))
                          {
                           *workString = strcom1.mid(11,ln);
                           strcom1=strcom1.right(strcom1.length()-ln);
                           calcKS =KS(workString->data(),ln);
                           if (calcKS==readKS)
                           {
                            i=0;
                            k=0;
                         for(j=0;j<71;j++)
                         {
                            k=i;
                            i=workString->indexOf(" ",k+1);



                            if (j<32) // Analogovie signaly
                            {
                                 flo =workString->mid(k,i-k).toFloat();
                                 if (j<12)
                                 {
                                     elem->Anlg[j]=flo;
                                 }else
                                 {
                                     elem->SgnAnl[j-12]=flo;
                                 }
                            }



                            if (j>=32 && j<=35) // Vihodnie signaly OutCww
                            {
                               bt =workString->mid(k,i-k).toInt();
                               if (j<34)
                               {
                                elem->OutCww1[j-32]=bt;
                               }else
                               {
                                elem->OutCww3[j-34]=bt;
                               }
                            }



                            if (j>=36 && j<=47) // Vhodnie signaly InCww
                            {
                               bt =workString->mid(k,i-k).toInt();
                               if (j<40)
                               {
                                elem->InCww1[j-36]=bt;
                               }else
                               { if(j<44)
                                 {
                                   elem->InCww2[j-40]=bt;
                                 }else
                                 {
                                    elem->InCww3[j-44]=bt;

                                 }
                               }
                            }



                            if (j>=48 && j<=67) // Dostovernost' 12ti analogovih signalov
                            {
                               bt =workString->mid(k,i-k).toInt();
                                elem->dsAn[j-48]=bt;
                            }


                            if (j==68) // ValUSO
                            {
                               bt =workString->mid(k,i-k).toInt();
                                elem->ValUSO = bt;
                            }

                            if (j==69) // ValMK
                            {
                               bt =workString->mid(k,i-k).toInt();
                                elem->ValMK=bt;
                            }



                            if (j==70) // Vremya
                            {int x=0;
                                for(x=0;x<12;x++) elem->time[x]=workString->mid(k+x+1,1).at(0).unicode();
                                emit NewRecReadyMK1(*elem);
                            }


                           }//for i=0 to 70
                          }
                 }

            }

        }

         pWork1=false;
        }

#ifdef DEBUGSHOWDATA
    qDebug("   ShowData newtxt1 end");
#endif
}

void ShowData::on_new_txt2(QString *data)
{ QString *buf=new  QString("\005\013\177");
    QString *workString=new QString;
    qint16 first=-1;
    quint16 ln=0;
    qint16 calcKS=0;
    qint16 readKS=0;
    qint16 i,j,k;
    float flo;
    int bt;
     RecordBlackBox *elem= new RecordBlackBox;

#ifdef DEBUGSHOWDATA
    qDebug("   ShowData newtxt2 start");
#endif

    if ((data->length() + strcom2.length())>3500 )
    {
        strcom2.clear();
    }
    strcom2.append(*data);

    if (!pWork2)
        {pWork2=true;

        first=strcom2.indexOf(*buf);
        if (first>-1)
            {

                 if ((strcom2.length()-first)<=strcom2.length())
                   strcom2=strcom2.right( strcom2.length()-first);

                 if (strcom2.length()>=11)
                 {
                      ln =Trans4in2( strcom2.mid(7,4).data());
                      readKS =Trans4in2( strcom2.mid(3,4).data());
                      if (strcom2.length()>=(11+ln))
                          {
                           *workString = strcom2.mid(11,ln);
                           strcom2=strcom2.right(strcom2.length()-ln);
                           calcKS =KS(workString->data(),ln);
                           if (calcKS==readKS)
                           {
                            i=0;
                            k=0;
                            for(j=0;j<71;j++)
                            {
                             k=i;
                             i=workString->indexOf(" ",k+1);



                             if (j<32) // Analogovie signaly
                             {
                                  flo =workString->mid(k,i-k).toFloat();
                                  if (j<12)
                                  {
                                      elem->Anlg[j]=flo;
                                  }else
                                  {
                                      elem->SgnAnl[j-12]=flo;
                                  }
                             }



                             if (j>=32 && j<=36) // Vihodnie signaly OutCww
                             {
                                bt =workString->mid(k,i-k).toInt();
                                if (j<34)
                                {
                                 elem->OutCww1[j-32]=bt;
                                }else
                                {
                                 elem->OutCww3[j-34]=bt;
                                }
                             }



                             if (j>=36 && j<=47) // Vhodnie signaly InCww
                             {
                                bt =workString->mid(k,i-k).toInt();
                                if (j<40)
                                {
                                 elem->InCww1[j-36]=bt;
                                }else
                                { if(j<44)
                                  {
                                    elem->InCww2[j-40]=bt;
                                  }else
                                  {
                                     elem->InCww3[j-44]=bt;

                                  }
                                }
                             }





                             if (j>=48 && j<=67) // Dostovernost' 12ti analogovih signalov
                             {
                                bt =workString->mid(k,i-k).toInt();
                                 elem->dsAn[j-48]=bt;
                             }



                             if (j==68) // ValUSO
                             {
                                bt =workString->mid(k,i-k).toInt();
                                 elem->ValUSO = bt;
                             }



                             if (j==69) // ValMK
                             {
                                bt =workString->mid(k,i-k).toInt();
                                 elem->ValMK=bt;
                             }



                             if (j==70)
                             {int x=0;
                                 for(x=0;x<12;x++) elem->time[x]=workString->mid(k+x+1,1).at(0).unicode();
                                emit NewRecReadyMK2(*elem);
                             }



                            }//for i=0 to 70



                           }
                          }
                 }

            }



         pWork2=false;
        }
#ifdef DEBUGSHOWDATA
    qDebug("   ShowData newtxt2 end");
#endif
}
void ShowData::on_new_txt3(QString *data)
{ QString *buf=new  QString("\005\013\177");
    QString *workString=new QString;
    qint16 first=-1;
    quint16 ln=0;
    qint16 calcKS=0;
    qint16 readKS=0;
    qint16 i,j,k;
    float flo;
    int bt;
    RecordBlackBox *elem =new RecordBlackBox;

#ifdef DEBUGSHOWDATA
    qDebug("   ShowData newtxt3 start");
#endif
    if ((data->length() + strcom3.length())>3500 )
    {
        strcom3.clear();
    }
    strcom3.append(*data);

    if (!pWork3)
        {pWork3=true;

        first=strcom3.indexOf(*buf);
        if (first>-1)
            {

                 if ((strcom3.length()-first)<=strcom3.length())
                   strcom3=strcom3.right( strcom3.length()-first);

                 if (strcom3.length()>=11)
                 {
                      ln =Trans4in2( strcom3.mid(7,4).data());
                      readKS =Trans4in2( strcom3.mid(3,4).data());
                      if (strcom3.length()>=(11+ln))
                          {
                           *workString = strcom3.mid(11,ln);
                           strcom3=strcom3.right(strcom3.length()-ln);
                           calcKS =KS(workString->data(),ln);
                           if (calcKS==readKS)
                           {
                            i=0;
                            k=0;
                            for(j=0;j<71;j++)
                            {
                             k=i;
                             i=workString->indexOf(" ",k+1);



                             if (j<32) // Analogovie signaly
                             {
                                  flo =workString->mid(k,i-k).toFloat();
                                  if (j<12)
                                  {
                                      elem->Anlg[j]=flo;
                                  }else
                                  {
                                      elem->SgnAnl[j-12]=flo;
                                  }
                             }



                             if (j>=32 && j<=35) // Vihodnie signaly OutCww
                             {
                                bt =workString->mid(k,i-k).toInt();
                                if (j<34)
                                {
                                 elem->OutCww1[j-32]=bt;
                                }else
                                {
                                 elem->OutCww3[j-34]=bt;
                                }
                             }



                             if (j>=36 && j<=47) // Vhodnie signaly InCww
                             {
                                bt =workString->mid(k,i-k).toInt();
                                if (j<40)
                                {
                                 elem->InCww1[j-36]=bt;
                                }else
                                { if(j<44)
                                  {
                                    elem->InCww2[j-40]=bt;
                                  }else
                                  {
                                     elem->InCww3[j-44]=bt;

                                  }
                                }
                             }



                             if (j>=48 && j<=67) // Dostovernost' 12ti analogovih signalov
                             {
                                bt =workString->mid(k,i-k).toInt();
                                 elem->dsAn[j-48]=bt;
                             }


                             if (j==68) // ValUSO
                             {
                                bt =workString->mid(k,i-k).toInt();
                                 elem->ValUSO = bt;
                             }



                             if (j==69) // ValMK
                             {
                                bt =workString->mid(k,i-k).toInt();
                                 elem->ValMK=bt;
                             }



                             if (j==70)
                             {   int x=0;
                                 for(x=0;x<12;x++)
                                     elem->time[x]=workString->mid(k+x+1,1).at(0).unicode();
                                 emit NewRecReadyMK3(*elem);
                             }


                            }//for i=0 to 55



                           }
                          }
                 }

            }



         pWork3=false;
        }
#ifdef DEBUGSHOWDATA
    qDebug("   ShowData newtxt3 end");
#endif
}

void ShowData::on_new_txt4(QString *data)
{ QString *buf=new  QString("\005\013\177");
    QString *workString=new QString;
    qint16 first=-1;
    quint16 ln=0;
    qint16 calcKS=0;
    qint16 readKS=0;
    qint16 i,j,k;
    float flo;
    int bt;
    RecordBlackBox *elem =new RecordBlackBox;

#ifdef DEBUGSHOWDATA
    qDebug("   ShowData newtxt4 start");
#endif
    if ((data->length() + strcom4.length())>3500 )
    {
        strcom4.clear();
    }
    strcom4.append(*data);

    if (!pWork4)
        {pWork4=true;

        first=strcom4.indexOf(*buf);
        if (first>-1)
            {

                 if ((strcom4.length()-first)<=strcom4.length())
                   strcom4=strcom4.right( strcom4.length()-first);

                 if (strcom4.length()>=11)
                 {
                      ln =Trans4in2( strcom4.mid(7,4).data());
                      readKS =Trans4in2( strcom4.mid(3,4).data());
                      if (strcom4.length()>=(11+ln))
                          {
                           *workString = strcom4.mid(11,ln);
                           strcom4=strcom4.right(strcom4.length()-ln);
                           calcKS =KS(workString->data(),ln);
                           if (calcKS==readKS)
                           {
                            i=0;
                            k=0;
                            for(j=0;j<11;j++)
                            {
                             k=i;
                             i=workString->indexOf(" ",k+1);





                             if (j>=0 && j<=3) // Vhodnie signaly InCww1
                             {
                                bt =workString->mid(k,i-k).toInt();

                                 elem->InCww1[j]=bt;

                             }


                             if (j>=4 && j<=7) // Vhodnie signaly InCww2
                             {
                                bt =workString->mid(k,i-k).toInt();

                                 elem->InCww2[j-4]=bt;

                             }


                             if (j==8) // ValUSO
                             {
                                bt =workString->mid(k,i-k).toInt();
                                 elem->ValUSO = bt;
                             }



                             if (j==9) // ValMK
                             {
                                bt =workString->mid(k,i-k).toInt();
                                 elem->ValMK=bt;
                             }



                             if (j==10)
                             {   int x=0;
                                 for(x=0;x<12;x++)
                                     elem->time[x]=workString->mid(k+x+1,1).at(0).unicode();
                                 emit NewRecReadyMK4(*elem);
                             }


                            }//for i=0 to 55



                           }
                          }
                 }

            }



         pWork4=false;
        }
#ifdef DEBUGSHOWDATA
    qDebug("   ShowData newtxt4 end");
#endif
}

qint16 ShowData::Trans4in2 (QChar *pb)
{
 // char ascii[] = "0123456789abcdefABCDEF";
  qint16 bb=0;
  int i,j;
  int roll=0;

  for(j=0;j<4;j++)
  {
   i=pb->unicode();
   if (i>=48 && i<58) i-=48; // ascii[] = "0123456789";
   if (i>=97 && i<103) i-=87; // ascii[] = "abcdef";
   if (i>=65 && i<71) i-=55; // ascii[] = "ABCDEF";
   bb|=(i<<(12-roll));
   pb++;
   roll+=4;
   }
   return(bb);
}


qint16 ShowData::KS (QChar *ptr, qint16 len)
{
 qint16 summa, i;

  summa=0;
  for (i=0; i<len; i++)
   { if (summa > summa+(ptr+i)->unicode()) summa++; summa+=(ptr+i)->unicode(); }
  return (summa);
}


/*
void Trans1in2 (BYTE bb, char * buf)
{
  char ascii[] = "0123456789abcdefABCDEF01345";
  int i;
   i=(bb & 0xf0) >> 4; *buf=ascii[i];
   i=bb & 0xf; *(buf+1)=ascii[i];
}


void Trans2in4 (WORD ss, char * buf)
{
  char ascii[] = "0123456789abcdefABCDEF01345";
  char i;
   i=(ss & 0xf000) >> 12; *buf=ascii[i];
   i=(ss & 0xf00) >> 8; *(buf+1)=ascii[i];
   i=(ss & 0xf0) >> 4; *(buf+2)=ascii[i];
   i=ss & 0xf; *(buf+3)=ascii[i];
}

unsigned char ShowData::Trans2in1 (char * pb)
{
  char ascii[] = "0123456789abcdefABCDEF01345";
  BYTE bb;
  int i;
   i=0; while (*pb!=ascii[i] && i<25) i++;
   if (i == 25) return 0; if (i > 15) i-=6; bb=(i << 4) & 0xf0;
   i=0; while (*(pb+1)!=ascii[i] && i<25) i++;
   if (i == 25) return 0; if (i > 15) i-=6; bb|=(i & 0x0f);
   return bb;
}
*/
