#include "mainwidget.h"
#include "ui_form.h"
#include "clientsocketadapter.h"
#include "dataproc/showdata.h"
#include "dataproc/recordstorage.h"
#include "dataproc/failstorage.h"
#include "util.h"


#ifdef DEBUGALL
#include <QScopedPointer>
#include <QDebug>
#include <QFile>

QScopedPointer<QFile> m_logFile;

void messageHandler(QtMsgType type,const char *msg);
#endif

//**********************************Основная процедура Пульт АРМ7К2
MainWidget::MainWidget(QWidget *parent) :
  QWidget(parent), m_pForm(new Ui::Form()) {

    qint8 ret;

    ADMINPASS = "3232";
    userName= getenv("USER");
    homeDir = "/home/" + userName.toLatin1();

#ifdef DEBUGALL
    m_logFile.reset(new QFile(homeDir + "/pultQTApp.log"));
    m_logFile.data()->open(QFile::Append|QFile::Text);
    //qInstallMsgHandler(messageHandler);
#endif

#ifdef DEBUGALL
    qDebug("   MainWidget Start");
#endif

    QDir di(homeDir + "/archives");
    if (!di.exists())
        ret = system("mkdir " + homeDir.toLatin1() + "/archives");

    di.setPath(homeDir + "/database");
    if (!di.exists())
        ret = system("mkdir " + homeDir.toLatin1() + "/database");

    /*  codec= QTextCodec::codecForName("utf-8");
  QTextCodec::setCodecForTr(codec);
  QTextCodec::setCodecForCStrings(codec);
  QTextCodec::setCodecForLocale(codec);
*/
  m_pForm->setupUi(this);
  m_pSock1 =new ClientSocketAdapter(this,1);
  m_pSock2 =new ClientSocketAdapter(this,2);
  m_pSock3 =new ClientSocketAdapter(this,3);  
  m_pSock4 =new ClientSocketAdapter(this,4);
  m_pShowdata = new ShowData(this);
  m_pRecordStorage = new RecordStorage(this,RECNUM,homeDir);
  failStorage = new FailStorage(this,12000,homeDir);

  avtWidget = 0;

 /* codec= QTextCodec::codecForName("utf-8");
  QTextCodec::setCodecForTr(codec);
  QTextCodec::setCodecForCStrings(codec);
  QTextCodec::setCodecForLocale(codec);
*/

  timer500mls = new QTimer(this);
  connect(timer500mls, SIGNAL(timeout()), this, SLOT(updateTimer500mls()));
  timer500mls->start(500);

  needSendTime = 3;

  timeRecArchive1 = QDateTime::currentDateTime().time().hour();
  timeRecArchive2 = QDateTime::currentDateTime().time().hour();
  timeRecArchive3 = QDateTime::currentDateTime().time().hour();
  timeRecArchive4 = QDateTime::currentDateTime().time().hour();
  archIsRared = false;
  archIsSaved = false;

  wasDisconnectMK1=false;
  wasDisconnectMK2=false;
  wasDisconnectMK3=false;
  wasDisconnectMK4=false;

  connect(m_pSock1, SIGNAL(message(QString *)), m_pShowdata,SLOT(on_new_txt1(QString *)));
  connect(m_pSock2, SIGNAL(message(QString *)), m_pShowdata,SLOT(on_new_txt2(QString *)));
  connect(m_pSock3, SIGNAL(message(QString *)), m_pShowdata,SLOT(on_new_txt3(QString *)));
  connect(m_pSock4, SIGNAL(message(QString *)), m_pShowdata,SLOT(on_new_txt4(QString *)));

  connect(m_pShowdata, SIGNAL(NewRecReadyMK1(RecordBlackBox )), SLOT(on_new_element_mk1(RecordBlackBox )));
  connect(m_pShowdata, SIGNAL(NewRecReadyMK2(RecordBlackBox )), SLOT(on_new_element_mk2(RecordBlackBox )));
  connect(m_pShowdata, SIGNAL(NewRecReadyMK3(RecordBlackBox )), SLOT(on_new_element_mk3(RecordBlackBox )));
  connect(m_pShowdata, SIGNAL(NewRecReadyMK4(RecordBlackBox )), SLOT(on_new_element_mk4(RecordBlackBox )));

  connect(m_pForm->actionSaveToFlash,SIGNAL(triggered()),SLOT(slotSaveToFlash()));

  connect(m_pForm->actionShowGraphWindow,SIGNAL(triggered()),SLOT(slotShowArchive()));

  connect(m_pForm->InSigButt_1,SIGNAL(clicked()),SLOT(slotShowArchive()));
  connect(m_pForm->InSigButt_2,SIGNAL(clicked()),SLOT(slotSaveToFlash()));
  connect(m_pForm->InSigButt_3,SIGNAL(clicked()),SLOT(slotExit()));
  connect(m_pForm->InSigButt_4,SIGNAL(clicked()),SLOT(slotSendOnOut()));
  connect(m_pForm->InSigButt_5,SIGNAL(clicked()),SLOT(slotShowGraphicCh1()));
  connect(m_pForm->InSigButt_6,SIGNAL(clicked()),SLOT(slotShowGraphicCh2()));
  connect(m_pForm->InSigButt_7,SIGNAL(clicked()),SLOT(slotShowGraphicCh3()));
  connect(m_pForm->InSigButt_8,SIGNAL(clicked()),SLOT(slotShowAvtWidget()));
  //connect(m_pForm->InSigButt_9,SIGNAL(clicked()),SLOT(slotShowUstWidget()));    устройства
 // connect(m_pForm->InSigButt_10,SIGNAL(clicked()),SLOT(slotShowCommonJourWidget()));   журналы О
 // connect(m_pForm->InSigButt_11,SIGNAL(clicked()),SLOT(slotShowLocalJourWidget()));    журналы Л
  connect(m_pForm->InSigButt_12,SIGNAL(clicked()),SLOT(slotShowSpaceMess()));

  connect(m_pForm->passButt_0,SIGNAL(pressed()),SLOT(slotPassButt0()));
  connect(m_pForm->passButt_1,SIGNAL(pressed()),SLOT(slotPassButt1()));
  connect(m_pForm->passButt_2,SIGNAL(pressed()),SLOT(slotPassButt2()));
  connect(m_pForm->passButt_3,SIGNAL(pressed()),SLOT(slotPassButt3()));
  connect(m_pForm->passButt_4,SIGNAL(pressed()),SLOT(slotPassButt4()));
  connect(m_pForm->passButt_5,SIGNAL(pressed()),SLOT(slotPassButt5()));
  connect(m_pForm->passButt_6,SIGNAL(pressed()),SLOT(slotPassButt6()));
  connect(m_pForm->passButt_7,SIGNAL(pressed()),SLOT(slotPassButt7()));
  connect(m_pForm->passButt_8,SIGNAL(pressed()),SLOT(slotPassButt8()));
  connect(m_pForm->passButt_9,SIGNAL(pressed()),SLOT(slotPassButt9()));
  connect(m_pForm->passButt_10,SIGNAL(pressed()),SLOT(slotPassButt10()));
  connect(m_pForm->passButt_11,SIGNAL(pressed()),SLOT(slotPassButt11()));

  m_pForm->groupBoxInpSIg->setVisible(true);
  m_pForm->menuBar->setVisible(false);
  m_pForm->framePassword->setVisible(false);
  m_pForm->InSigButt_1->setVisible(false);

  enterPassword ="";
  passAction =0;

  isRedrawingMainWindowData = true;

  AnIn[0] = m_pForm->_AnIn_0_0; AnIn[1] = m_pForm->_AnIn_0_1; AnIn[2] = m_pForm->_AnIn_0_2;
  AnIn[3] = m_pForm->_AnIn_0_3; AnIn[4] = m_pForm->_AnIn_0_4; AnIn[5] = m_pForm->_AnIn_0_5;
  AnIn[6] = m_pForm->_AnIn_0_6; AnIn[7] = m_pForm->_AnIn_0_7; AnIn[8] = m_pForm->_AnIn_0_8;
  AnIn[9] = m_pForm->_AnIn_0_9; AnIn[10] = m_pForm->_AnIn_0_10; AnIn[11] = m_pForm->_AnIn_0_11;
  AnIn[12] = m_pForm->_AnIn_0_12; AnIn[13] = m_pForm->_AnIn_0_13; AnIn[14] = m_pForm->_AnIn_0_14;
  AnIn[15] = m_pForm->_AnIn_0_15; AnIn[16] = m_pForm->_AnIn_0_16; AnIn[17] = m_pForm->_AnIn_0_17;
  AnIn[18] = m_pForm->_AnIn_0_18; AnIn[19] = m_pForm->_AnIn_0_19; AnIn[20] = m_pForm->_AnIn_0_20;
  AnIn[21] = m_pForm->_AnIn_0_21; AnIn[22] = m_pForm->_AnIn_0_22; AnIn[23] = m_pForm->_AnIn_0_23;
  AnIn[24] = m_pForm->_AnIn_0_24; AnIn[25] = m_pForm->_AnIn_0_25; AnIn[26] = m_pForm->_AnIn_0_26;
  AnIn[27] = m_pForm->_AnIn_0_27; AnIn[28] = m_pForm->_AnIn_0_28; AnIn[29] = m_pForm->_AnIn_0_29;
  AnIn[30] = m_pForm->_AnIn_0_30; AnIn[31] = m_pForm->_AnIn_0_31; AnIn[32] = m_pForm->_AnIn_0_32;
  AnIn[33] = m_pForm->_AnIn_0_33; AnIn[34] = m_pForm->_AnIn_0_34; AnIn[35] = m_pForm->_AnIn_0_35;
  AnIn[36] = m_pForm->_AnIn_0_36; AnIn[37] = m_pForm->_AnIn_0_37; AnIn[38] = m_pForm->_AnIn_0_38;
  AnIn[39] = m_pForm->_AnIn_0_39; AnIn[40] = m_pForm->_AnIn_0_40; AnIn[41] = m_pForm->_AnIn_0_41;
  AnIn[42] = m_pForm->_AnIn_0_42; AnIn[43] = m_pForm->_AnIn_0_43; AnIn[44] = m_pForm->_AnIn_0_44;
  AnIn[45] = m_pForm->_AnIn_0_45; AnIn[46] = m_pForm->_AnIn_0_46; AnIn[47] = m_pForm->_AnIn_0_47;
  AnIn[48] = m_pForm->_AnIn_0_48; AnIn[49] = m_pForm->_AnIn_0_49; AnIn[50] = m_pForm->_AnIn_0_50;
  AnIn[51] = m_pForm->_AnIn_0_51; AnIn[52] = m_pForm->_AnIn_0_52; AnIn[53] = m_pForm->_AnIn_0_53;
  AnIn[54] = m_pForm->_AnIn_0_54; AnIn[55] = m_pForm->_AnIn_0_55; AnIn[56] = m_pForm->_AnIn_0_56;
  AnIn[57] = m_pForm->_AnIn_0_57; AnIn[58] = m_pForm->_AnIn_0_58; AnIn[59] = m_pForm->_AnIn_0_59;

  dsAn[0] = m_pForm->_dsAn_0_0; dsAn[1] = m_pForm->_dsAn_0_1; dsAn[2] = m_pForm->_dsAn_0_2;
  dsAn[3] = m_pForm->_dsAn_0_3; dsAn[4] = m_pForm->_dsAn_0_4; dsAn[5] = m_pForm->_dsAn_0_5;
  dsAn[6] = m_pForm->_dsAn_0_6; dsAn[7] = m_pForm->_dsAn_0_7; dsAn[8] = m_pForm->_dsAn_0_8;
  dsAn[9] = m_pForm->_dsAn_0_9; dsAn[10] = m_pForm->_dsAn_0_10; dsAn[11] = m_pForm->_dsAn_0_11;
  dsAn[12] = m_pForm->_dsAn_0_12; dsAn[13] = m_pForm->_dsAn_0_13; dsAn[14] = m_pForm->_dsAn_0_14;
  dsAn[15] = m_pForm->_dsAn_0_15; dsAn[16] = m_pForm->_dsAn_0_16; dsAn[17] = m_pForm->_dsAn_0_17;
  dsAn[18] = m_pForm->_dsAn_0_18; dsAn[19] = m_pForm->_dsAn_0_19; dsAn[20] = m_pForm->_dsAn_0_20;
  dsAn[21] = m_pForm->_dsAn_0_21; dsAn[22] = m_pForm->_dsAn_0_22; dsAn[23] = m_pForm->_dsAn_0_23;
  dsAn[24] = m_pForm->_dsAn_0_24; dsAn[25] = m_pForm->_dsAn_0_25; dsAn[26] = m_pForm->_dsAn_0_26;
  dsAn[27] = m_pForm->_dsAn_0_27; dsAn[28] = m_pForm->_dsAn_0_28; dsAn[29] = m_pForm->_dsAn_0_29;
  dsAn[30] = m_pForm->_dsAn_0_30; dsAn[31] = m_pForm->_dsAn_0_31; dsAn[32] = m_pForm->_dsAn_0_32;
  dsAn[33] = m_pForm->_dsAn_0_33; dsAn[34] = m_pForm->_dsAn_0_34; dsAn[35] = m_pForm->_dsAn_0_35;
  dsAn[36] = m_pForm->_dsAn_0_36; dsAn[37] = m_pForm->_dsAn_0_37; dsAn[38] = m_pForm->_dsAn_0_38;
  dsAn[39] = m_pForm->_dsAn_0_39; dsAn[40] = m_pForm->_dsAn_0_40; dsAn[41] = m_pForm->_dsAn_0_41;
  dsAn[42] = m_pForm->_dsAn_0_42; dsAn[43] = m_pForm->_dsAn_0_43; dsAn[44] = m_pForm->_dsAn_0_44;
  dsAn[45] = m_pForm->_dsAn_0_45; dsAn[46] = m_pForm->_dsAn_0_46; dsAn[47] = m_pForm->_dsAn_0_47;
  dsAn[48] = m_pForm->_dsAn_0_48; dsAn[49] = m_pForm->_dsAn_0_49; dsAn[50] = m_pForm->_dsAn_0_50;
  dsAn[51] = m_pForm->_dsAn_0_51; dsAn[52] = m_pForm->_dsAn_0_52; dsAn[53] = m_pForm->_dsAn_0_53;
  dsAn[54] = m_pForm->_dsAn_0_54; dsAn[55] = m_pForm->_dsAn_0_55; dsAn[56] = m_pForm->_dsAn_0_56;
  dsAn[57] = m_pForm->_dsAn_0_57; dsAn[58] = m_pForm->_dsAn_0_58; dsAn[59] = m_pForm->_dsAn_0_59;

  DsMas[0]= m_pForm->_dsIn_0_0;     DsMas[96]= m_pForm->_dsIn_1_0;      DsMas[192]= m_pForm->_dsIn_2_0;
  DsMas[1]= m_pForm->_dsIn_0_1;     DsMas[97]= m_pForm->_dsIn_1_1;      DsMas[193]= m_pForm->_dsIn_2_1;
  DsMas[2]= m_pForm->_dsIn_0_2;     DsMas[98]= m_pForm->_dsIn_1_2;      DsMas[194]= m_pForm->_dsIn_2_2;
  DsMas[3]= m_pForm->_dsIn_0_3;     DsMas[99]= m_pForm->_dsIn_1_3;      DsMas[195]= m_pForm->_dsIn_2_3;
  DsMas[4]= m_pForm->_dsIn_0_4;     DsMas[100]= m_pForm->_dsIn_1_4;      DsMas[196]= m_pForm->_dsIn_2_4;
  DsMas[5]= m_pForm->_dsIn_0_5;     DsMas[101]= m_pForm->_dsIn_1_5;      DsMas[197]= m_pForm->_dsIn_2_5;
  DsMas[6]= m_pForm->_dsIn_0_6;     DsMas[102]= m_pForm->_dsIn_1_6;      DsMas[198]= m_pForm->_dsIn_2_6;
  DsMas[7]= m_pForm->_dsIn_0_7;     DsMas[103]= m_pForm->_dsIn_1_7;      DsMas[199]= m_pForm->_dsIn_2_7;
  DsMas[8]= m_pForm->_dsIn_0_8;     DsMas[104]= m_pForm->_dsIn_1_8;      DsMas[200]= m_pForm->_dsIn_2_8;
  DsMas[9]= m_pForm->_dsIn_0_9;     DsMas[105]= m_pForm->_dsIn_1_9;      DsMas[201]= m_pForm->_dsIn_2_9;
  DsMas[10]= m_pForm->_dsIn_0_10;   DsMas[106]= m_pForm->_dsIn_1_10;      DsMas[202]= m_pForm->_dsIn_2_10;
  DsMas[11]= m_pForm->_dsIn_0_11;   DsMas[107]= m_pForm->_dsIn_1_11;      DsMas[203]= m_pForm->_dsIn_2_11;
  DsMas[12]= m_pForm->_dsIn_0_12;   DsMas[108]= m_pForm->_dsIn_1_12;      DsMas[204]= m_pForm->_dsIn_2_12;
  DsMas[13]= m_pForm->_dsIn_0_13;   DsMas[109]= m_pForm->_dsIn_1_13;      DsMas[205]= m_pForm->_dsIn_2_13;
  DsMas[14]= m_pForm->_dsIn_0_14;   DsMas[110]= m_pForm->_dsIn_1_14;      DsMas[206]= m_pForm->_dsIn_2_14;
  DsMas[15]= m_pForm->_dsIn_0_15;   DsMas[111]= m_pForm->_dsIn_1_15;      DsMas[207]= m_pForm->_dsIn_2_15;

  DsMas[16]= m_pForm->_dsIn_0_16;    DsMas[112]= m_pForm->_dsIn_1_16;      DsMas[208]= m_pForm->_dsIn_2_16;
  DsMas[17]= m_pForm->_dsIn_0_17;    DsMas[113]= m_pForm->_dsIn_1_17;      DsMas[209]= m_pForm->_dsIn_2_17;
  DsMas[18]= m_pForm->_dsIn_0_18;    DsMas[114]= m_pForm->_dsIn_1_18;      DsMas[210]= m_pForm->_dsIn_2_18;
  DsMas[19]= m_pForm->_dsIn_0_19;    DsMas[115]= m_pForm->_dsIn_1_19;      DsMas[211]= m_pForm->_dsIn_2_19;
  DsMas[20]= m_pForm->_dsIn_0_20;    DsMas[116]= m_pForm->_dsIn_1_20;      DsMas[212]= m_pForm->_dsIn_2_20;
  DsMas[21]= m_pForm->_dsIn_0_21;    DsMas[117]= m_pForm->_dsIn_1_21;      DsMas[213]= m_pForm->_dsIn_2_21;
  DsMas[22]= m_pForm->_dsIn_0_22;    DsMas[118]= m_pForm->_dsIn_1_22;      DsMas[214]= m_pForm->_dsIn_2_22;
  DsMas[23]= m_pForm->_dsIn_0_23;    DsMas[119]= m_pForm->_dsIn_1_23;      DsMas[215]= m_pForm->_dsIn_2_23;
  DsMas[24]= m_pForm->_dsIn_0_24;    DsMas[120]= m_pForm->_dsIn_1_24;      DsMas[216]= m_pForm->_dsIn_2_24;
  DsMas[25]= m_pForm->_dsIn_0_25;    DsMas[121]= m_pForm->_dsIn_1_25;      DsMas[217]= m_pForm->_dsIn_2_25;
  DsMas[26]= m_pForm->_dsIn_0_26;    DsMas[122]= m_pForm->_dsIn_1_26;      DsMas[218]= m_pForm->_dsIn_2_26;
  DsMas[27]= m_pForm->_dsIn_0_27;    DsMas[123]= m_pForm->_dsIn_1_27;      DsMas[219]= m_pForm->_dsIn_2_27;
  DsMas[28]= m_pForm->_dsIn_0_28;    DsMas[124]= m_pForm->_dsIn_1_28;      DsMas[220]= m_pForm->_dsIn_2_28;
  DsMas[29]= m_pForm->_dsIn_0_29;    DsMas[125]= m_pForm->_dsIn_1_29;      DsMas[221]= m_pForm->_dsIn_2_29;
  DsMas[30]= m_pForm->_dsIn_0_30;    DsMas[126]= m_pForm->_dsIn_1_30;      DsMas[222]= m_pForm->_dsIn_2_30;
  DsMas[31]= m_pForm->_dsIn_0_31;    DsMas[127]= m_pForm->_dsIn_1_31;      DsMas[223]= m_pForm->_dsIn_2_31;

  DsMas[32]= m_pForm->_dsIn_0_32;    DsMas[128]= m_pForm->_dsIn_1_32;      DsMas[224]= m_pForm->_dsIn_2_32;
  DsMas[33]= m_pForm->_dsIn_0_33;    DsMas[129]= m_pForm->_dsIn_1_33;      DsMas[225]= m_pForm->_dsIn_2_33;
  DsMas[34]= m_pForm->_dsIn_0_34;    DsMas[130]= m_pForm->_dsIn_1_34;      DsMas[226]= m_pForm->_dsIn_2_34;
  DsMas[35]= m_pForm->_dsIn_0_35;    DsMas[131]= m_pForm->_dsIn_1_35;      DsMas[227]= m_pForm->_dsIn_2_35;
  DsMas[36]= m_pForm->_dsIn_0_36;    DsMas[132]= m_pForm->_dsIn_1_36;     DsMas[228]= m_pForm->_dsIn_2_36;
  DsMas[37]= m_pForm->_dsIn_0_37;    DsMas[133]= m_pForm->_dsIn_1_37;     DsMas[229]= m_pForm->_dsIn_2_37;
  DsMas[38]= m_pForm->_dsIn_0_38;    DsMas[134]= m_pForm->_dsIn_1_38;     DsMas[230]= m_pForm->_dsIn_2_38;
  DsMas[39]= m_pForm->_dsIn_0_39;    DsMas[135]= m_pForm->_dsIn_1_39;     DsMas[231]= m_pForm->_dsIn_2_39;
  DsMas[40]= m_pForm->_dsIn_0_40;    DsMas[136]= m_pForm->_dsIn_1_40;     DsMas[232]= m_pForm->_dsIn_2_40;
  DsMas[41]= m_pForm->_dsIn_0_41;    DsMas[137]= m_pForm->_dsIn_1_41;     DsMas[233]= m_pForm->_dsIn_2_41;
  DsMas[42]= m_pForm->_dsIn_0_42;    DsMas[138]= m_pForm->_dsIn_1_42;     DsMas[234]= m_pForm->_dsIn_2_42;
  DsMas[43]= m_pForm->_dsIn_0_43;    DsMas[139]= m_pForm->_dsIn_1_43;     DsMas[235]= m_pForm->_dsIn_2_43;
  DsMas[44]= m_pForm->_dsIn_0_44;    DsMas[140]= m_pForm->_dsIn_1_44;     DsMas[236]= m_pForm->_dsIn_2_44;
  DsMas[45]= m_pForm->_dsIn_0_45;    DsMas[141]= m_pForm->_dsIn_1_45;     DsMas[237]= m_pForm->_dsIn_2_45;
  DsMas[46]= m_pForm->_dsIn_0_46;    DsMas[142]= m_pForm->_dsIn_1_46;     DsMas[238]= m_pForm->_dsIn_2_46;
  DsMas[47]= m_pForm->_dsIn_0_47;    DsMas[143]= m_pForm->_dsIn_1_47;     DsMas[239]= m_pForm->_dsIn_2_47;

  DsMas[48]= m_pForm->_dsIn_0_48;    DsMas[144]= m_pForm->_dsIn_1_48;     DsMas[240]= m_pForm->_dsIn_2_48;
  DsMas[49]= m_pForm->_dsIn_0_49;    DsMas[145]= m_pForm->_dsIn_1_49;     DsMas[241]= m_pForm->_dsIn_2_49;
  DsMas[50]= m_pForm->_dsIn_0_50;    DsMas[146]= m_pForm->_dsIn_1_50;     DsMas[242]= m_pForm->_dsIn_2_50;
  DsMas[51]= m_pForm->_dsIn_0_51;    DsMas[147]= m_pForm->_dsIn_1_51;     DsMas[243]= m_pForm->_dsIn_2_51;
  DsMas[52]= m_pForm->_dsIn_0_52;    DsMas[148]= m_pForm->_dsIn_1_52;     DsMas[244]= m_pForm->_dsIn_2_52;
  DsMas[53]= m_pForm->_dsIn_0_53;    DsMas[149]= m_pForm->_dsIn_1_53;     DsMas[245]= m_pForm->_dsIn_2_53;
  DsMas[54]= m_pForm->_dsIn_0_54;    DsMas[150]= m_pForm->_dsIn_1_54;     DsMas[246]= m_pForm->_dsIn_2_54;
  DsMas[55]= m_pForm->_dsIn_0_55;    DsMas[151]= m_pForm->_dsIn_1_55;     DsMas[247]= m_pForm->_dsIn_2_55;
  DsMas[56]= m_pForm->_dsIn_0_56;    DsMas[152]= m_pForm->_dsIn_1_56;     DsMas[248]= m_pForm->_dsIn_2_56;
  DsMas[57]= m_pForm->_dsIn_0_57;    DsMas[153]= m_pForm->_dsIn_1_57;     DsMas[249]= m_pForm->_dsIn_2_57;
  DsMas[58]= m_pForm->_dsIn_0_58;    DsMas[154]= m_pForm->_dsIn_1_58;     DsMas[250]= m_pForm->_dsIn_2_58;
  DsMas[59]= m_pForm->_dsIn_0_59;    DsMas[155]= m_pForm->_dsIn_1_59;     DsMas[251]= m_pForm->_dsIn_2_59;
  DsMas[60]= m_pForm->_dsIn_0_60;    DsMas[156]= m_pForm->_dsIn_1_60;     DsMas[252]= m_pForm->_dsIn_2_60;
  DsMas[61]= m_pForm->_dsIn_0_61;    DsMas[157]= m_pForm->_dsIn_1_61;     DsMas[253]= m_pForm->_dsIn_2_61;
  DsMas[62]= m_pForm->_dsIn_0_62;    DsMas[158]= m_pForm->_dsIn_1_62;     DsMas[254]= m_pForm->_dsIn_2_62;
  DsMas[63]= m_pForm->_dsIn_0_63;    DsMas[159]= m_pForm->_dsIn_1_63;     DsMas[255]= m_pForm->_dsIn_2_63;

  DsMas[64]= m_pForm->_dsIn_0_64;    DsMas[160]= m_pForm->_dsIn_1_64;      DsMas[256]= m_pForm->_dsIn_2_64;
  DsMas[65]= m_pForm->_dsIn_0_65;    DsMas[161]= m_pForm->_dsIn_1_65;      DsMas[257]= m_pForm->_dsIn_2_65;
  DsMas[66]= m_pForm->_dsIn_0_66;    DsMas[162]= m_pForm->_dsIn_1_66;      DsMas[258]= m_pForm->_dsIn_2_66;
  DsMas[67]= m_pForm->_dsIn_0_67;    DsMas[163]= m_pForm->_dsIn_1_67;      DsMas[259]= m_pForm->_dsIn_2_67;
  DsMas[68]= m_pForm->_dsIn_0_68;    DsMas[164]= m_pForm->_dsIn_1_68;     DsMas[260]= m_pForm->_dsIn_2_68;
  DsMas[69]= m_pForm->_dsIn_0_69;    DsMas[165]= m_pForm->_dsIn_1_69;     DsMas[261]= m_pForm->_dsIn_2_69;
  DsMas[70]= m_pForm->_dsIn_0_70;    DsMas[166]= m_pForm->_dsIn_1_70;     DsMas[262]= m_pForm->_dsIn_2_70;
  DsMas[71]= m_pForm->_dsIn_0_71;    DsMas[167]= m_pForm->_dsIn_1_71;     DsMas[263]= m_pForm->_dsIn_2_71;
  DsMas[72]= m_pForm->_dsIn_0_72;    DsMas[168]= m_pForm->_dsIn_1_72;     DsMas[264]= m_pForm->_dsIn_2_72;
  DsMas[73]= m_pForm->_dsIn_0_73;    DsMas[169]= m_pForm->_dsIn_1_73;     DsMas[265]= m_pForm->_dsIn_2_73;
  DsMas[74]= m_pForm->_dsIn_0_74;    DsMas[170]= m_pForm->_dsIn_1_74;     DsMas[266]= m_pForm->_dsIn_2_74;
  DsMas[75]= m_pForm->_dsIn_0_75;    DsMas[171]= m_pForm->_dsIn_1_75;     DsMas[267]= m_pForm->_dsIn_2_75;
  DsMas[76]= m_pForm->_dsIn_0_76;    DsMas[172]= m_pForm->_dsIn_1_76;     DsMas[268]= m_pForm->_dsIn_2_76;
  DsMas[77]= m_pForm->_dsIn_0_77;    DsMas[173]= m_pForm->_dsIn_1_77;     DsMas[269]= m_pForm->_dsIn_2_77;
  DsMas[78]= m_pForm->_dsIn_0_78;    DsMas[174]= m_pForm->_dsIn_1_78;     DsMas[270]= m_pForm->_dsIn_2_78;
  DsMas[79]= m_pForm->_dsIn_0_79;    DsMas[175]= m_pForm->_dsIn_1_79;     DsMas[271]= m_pForm->_dsIn_2_79;

  DsMas[80]= m_pForm->_dsIn_0_80;    DsMas[176]= m_pForm->_dsIn_1_80;     DsMas[272]= m_pForm->_dsIn_2_80;
  DsMas[81]= m_pForm->_dsIn_0_81;    DsMas[177]= m_pForm->_dsIn_1_81;     DsMas[273]= m_pForm->_dsIn_2_81;
  DsMas[82]= m_pForm->_dsIn_0_82;    DsMas[178]= m_pForm->_dsIn_1_82;     DsMas[274]= m_pForm->_dsIn_2_82;
  DsMas[83]= m_pForm->_dsIn_0_83;    DsMas[179]= m_pForm->_dsIn_1_83;     DsMas[275]= m_pForm->_dsIn_2_83;
  DsMas[84]= m_pForm->_dsIn_0_84;    DsMas[180]= m_pForm->_dsIn_1_84;     DsMas[276]= m_pForm->_dsIn_2_84;
  DsMas[85]= m_pForm->_dsIn_0_85;    DsMas[181]= m_pForm->_dsIn_1_85;     DsMas[277]= m_pForm->_dsIn_2_85;
  DsMas[86]= m_pForm->_dsIn_0_86;    DsMas[182]= m_pForm->_dsIn_1_86;     DsMas[278]= m_pForm->_dsIn_2_86;
  DsMas[87]= m_pForm->_dsIn_0_87;    DsMas[183]= m_pForm->_dsIn_1_87;     DsMas[279]= m_pForm->_dsIn_2_87;
  DsMas[88]= m_pForm->_dsIn_0_88;    DsMas[184]= m_pForm->_dsIn_1_88;     DsMas[280]= m_pForm->_dsIn_2_88;
  DsMas[89]= m_pForm->_dsIn_0_89;    DsMas[185]= m_pForm->_dsIn_1_89;     DsMas[281]= m_pForm->_dsIn_2_89;
  DsMas[90]= m_pForm->_dsIn_0_90;    DsMas[186]= m_pForm->_dsIn_1_90;     DsMas[282]= m_pForm->_dsIn_2_90;
  DsMas[91]= m_pForm->_dsIn_0_91;    DsMas[187]= m_pForm->_dsIn_1_91;     DsMas[283]= m_pForm->_dsIn_2_91;
  DsMas[92]= m_pForm->_dsIn_0_92;    DsMas[188]= m_pForm->_dsIn_1_92;     DsMas[284]= m_pForm->_dsIn_2_92;
  DsMas[93]= m_pForm->_dsIn_0_93;    DsMas[189]= m_pForm->_dsIn_1_93;     DsMas[285]= m_pForm->_dsIn_2_93;
  DsMas[94]= m_pForm->_dsIn_0_94;    DsMas[190]= m_pForm->_dsIn_1_94;     DsMas[286]= m_pForm->_dsIn_2_94;
  DsMas[95]= m_pForm->_dsIn_0_95;    DsMas[191]= m_pForm->_dsIn_1_95;     DsMas[287]= m_pForm->_dsIn_2_95;

  DsOutMas[0] =m_pForm->_dsOut_0_0;     DsOutMas[32] =m_pForm->_dsOut_1_0;       DsOutMas[64] =m_pForm->_dsOut_2_0;
  DsOutMas[1] =m_pForm->_dsOut_0_1;     DsOutMas[33] =m_pForm->_dsOut_1_1;       DsOutMas[65] =m_pForm->_dsOut_2_1;
  DsOutMas[2] =m_pForm->_dsOut_0_2;     DsOutMas[34] =m_pForm->_dsOut_1_2;       DsOutMas[66] =m_pForm->_dsOut_2_2;
  DsOutMas[3] =m_pForm->_dsOut_0_3;     DsOutMas[35] =m_pForm->_dsOut_1_3;       DsOutMas[67] =m_pForm->_dsOut_2_3;
  DsOutMas[4] =m_pForm->_dsOut_0_4;     DsOutMas[36] =m_pForm->_dsOut_1_4;       DsOutMas[68] =m_pForm->_dsOut_2_4;
  DsOutMas[5] =m_pForm->_dsOut_0_5;     DsOutMas[37] =m_pForm->_dsOut_1_5;       DsOutMas[69] =m_pForm->_dsOut_2_5;
  DsOutMas[6] =m_pForm->_dsOut_0_6;     DsOutMas[38] =m_pForm->_dsOut_1_6;       DsOutMas[70] =m_pForm->_dsOut_2_6;
  DsOutMas[7] =m_pForm->_dsOut_0_7;     DsOutMas[39] =m_pForm->_dsOut_1_7;       DsOutMas[71] =m_pForm->_dsOut_2_7;
  DsOutMas[8] =m_pForm->_dsOut_0_8;     DsOutMas[40] =m_pForm->_dsOut_1_8;       DsOutMas[72] =m_pForm->_dsOut_2_8;
  DsOutMas[9] =m_pForm->_dsOut_0_9;     DsOutMas[41] =m_pForm->_dsOut_1_9;       DsOutMas[73] =m_pForm->_dsOut_2_9;
  DsOutMas[10] =m_pForm->_dsOut_0_10;     DsOutMas[42] =m_pForm->_dsOut_1_10;       DsOutMas[74] =m_pForm->_dsOut_2_10;
  DsOutMas[11] =m_pForm->_dsOut_0_11;     DsOutMas[43] =m_pForm->_dsOut_1_11;       DsOutMas[75] =m_pForm->_dsOut_2_11;
  DsOutMas[12] =m_pForm->_dsOut_0_12;     DsOutMas[44] =m_pForm->_dsOut_1_12;       DsOutMas[76] =m_pForm->_dsOut_2_12;
  DsOutMas[13] =m_pForm->_dsOut_0_13;     DsOutMas[45] =m_pForm->_dsOut_1_13;       DsOutMas[77] =m_pForm->_dsOut_2_13;
  DsOutMas[14] =m_pForm->_dsOut_0_14;     DsOutMas[46] =m_pForm->_dsOut_1_14;       DsOutMas[78] =m_pForm->_dsOut_2_14;
  DsOutMas[15] =m_pForm->_dsOut_0_15;     DsOutMas[47] =m_pForm->_dsOut_1_15;       DsOutMas[79] =m_pForm->_dsOut_2_15;
  DsOutMas[16] =m_pForm->_dsOut_0_16;     DsOutMas[48] =m_pForm->_dsOut_1_16;       DsOutMas[80] =m_pForm->_dsOut_2_16;
  DsOutMas[17] =m_pForm->_dsOut_0_17;     DsOutMas[49] =m_pForm->_dsOut_1_17;       DsOutMas[81] =m_pForm->_dsOut_2_17;
  DsOutMas[18] =m_pForm->_dsOut_0_18;     DsOutMas[50] =m_pForm->_dsOut_1_18;       DsOutMas[82] =m_pForm->_dsOut_2_18;
  DsOutMas[19] =m_pForm->_dsOut_0_19;     DsOutMas[51] =m_pForm->_dsOut_1_19;       DsOutMas[83] =m_pForm->_dsOut_2_19;
  DsOutMas[20] =m_pForm->_dsOut_0_20;     DsOutMas[52] =m_pForm->_dsOut_1_20;       DsOutMas[84] =m_pForm->_dsOut_2_20;
  DsOutMas[21] =m_pForm->_dsOut_0_21;     DsOutMas[53] =m_pForm->_dsOut_1_21;       DsOutMas[85] =m_pForm->_dsOut_2_21;
  DsOutMas[22] =m_pForm->_dsOut_0_22;     DsOutMas[54] =m_pForm->_dsOut_1_22;       DsOutMas[86] =m_pForm->_dsOut_2_22;
  DsOutMas[23] =m_pForm->_dsOut_0_23;     DsOutMas[55] =m_pForm->_dsOut_1_23;       DsOutMas[87] =m_pForm->_dsOut_2_23;
  DsOutMas[24] =m_pForm->_dsOut_0_24;     DsOutMas[56] =m_pForm->_dsOut_1_24;       DsOutMas[88] =m_pForm->_dsOut_2_24;
  DsOutMas[25] =m_pForm->_dsOut_0_25;     DsOutMas[57] =m_pForm->_dsOut_1_25;       DsOutMas[89] =m_pForm->_dsOut_2_25;
  DsOutMas[26] =m_pForm->_dsOut_0_26;     DsOutMas[58] =m_pForm->_dsOut_1_26;       DsOutMas[90] =m_pForm->_dsOut_2_26;
  DsOutMas[27] =m_pForm->_dsOut_0_27;     DsOutMas[59] =m_pForm->_dsOut_1_27;       DsOutMas[91] =m_pForm->_dsOut_2_27;
  DsOutMas[28] =m_pForm->_dsOut_0_28;     DsOutMas[60] =m_pForm->_dsOut_1_28;       DsOutMas[92] =m_pForm->_dsOut_2_28;
  DsOutMas[29] =m_pForm->_dsOut_0_29;     DsOutMas[61] =m_pForm->_dsOut_1_29;       DsOutMas[93] =m_pForm->_dsOut_2_29;
  DsOutMas[30] =m_pForm->_dsOut_0_30;     DsOutMas[62] =m_pForm->_dsOut_1_30;       DsOutMas[94] =m_pForm->_dsOut_2_30;
  DsOutMas[31] =m_pForm->_dsOut_0_31;     DsOutMas[63] =m_pForm->_dsOut_1_31;       DsOutMas[95] =m_pForm->_dsOut_2_31;

 //connect(m_pForm->send, SIGNAL(clicked()), SLOT(on_send_mk1()));
}

//****************************************************************************************
void MainWidget::archiveBase()
{
    qint8 ret;

    QString sourceDir =homeDir + "/database/";
    QString destDir =homeDir + "/archives/";
    QString com="cd " + sourceDir + " && tar -czf " + destDir +"Arch[`date +%d`.`date +%m`.`date +%Y`].tar " + "./bak2*.* --remove-files";

#ifdef DEBUGALL
    qDebug() << "archiveBase START " << ret;
#endif
    ret = system(com.toLatin1().constData());

    //ret = QProcess::execute(com.toAscii().constData());

#ifdef DEBUGALL
    qDebug() << "archiveBase STOP " << ret;
#endif

}

//****************************************************************************************
void MainWidget::cleanArchives()
{
    QDir dirObj(homeDir +"/archives");
    QFileInfo fileInfo;
    quint64 si=0;

      foreach(QString fileName,dirObj.entryList())
      {
          if (fileName!="." && fileName!="..")
          {
             fileInfo.setFile(dirObj.absolutePath() + '/' + fileName);
             si  += fileInfo.size();
          }
      }

     if ( si>((MAX_ARCHIVES_IN_MB -500 )* 1024 * 1024) ) //sravnenie v baytah
     {
          m_pForm->InSigButt_12->setVisible(true);
     }
     else
     {     m_pForm->InSigButt_12->setVisible(false); }

     if ( si>(MAX_ARCHIVES_IN_MB * 1024 * 1024) ) //sravnenie v baytah
     {
           foreach(QString fileName,dirObj.entryList())
           {
             if (fileName!="." && fileName!="..")
             {
               dirObj.remove(fileName);
             }
           }
     }
}

//****************************************************************************************
void MainWidget::clearGraphWindow(qint8 chan)
{
    qint8 i=0;

    switch (chan) {
    case 1:
        m_pForm->lab_txt_mk1_0->setText("");
        m_pForm->lab_txt_mk1_1->setText("");
        m_pForm->lab_txt_mk1_2->setText("");
        m_pForm->lab_txt_mk1_3->setText("");
        m_pForm->lab_txt_mk1_4->setText("");
        m_pForm->lab_txt_mk1_5->setText("");
        m_pForm->lab_txt_mk1_6->setText("");
        m_pForm->lab_txt_mk1_7->setText("");
        m_pForm->lab_txt_mk1_8->setText("");
        m_pForm->lab_txt_mk1_9->setText("");
        m_pForm->lab_txt_mk1_10->setText("");
        m_pForm->lab_txt_mk1_11->setText("");
        m_pForm->lab_txt_mk1_12->setText("");
        m_pForm->lab_txt_mk1_13->setText("");
        m_pForm->lab_txt_mk1_14->setText("");

        m_pForm->lImit_mk1->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb( 239, 239, 239)");
 //       m_pForm->lDhcp_mk1->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb( 239, 239, 239)");
        m_pForm->lRs232_mk1->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb( 239, 239, 239)");

        m_pForm->lab_sig_mk1_0->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk1_1->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk1_2->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk1_3->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk1_4->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk1_5->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk1_6->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk1_7->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk1_8->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk1_9->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk1_10->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk1_11->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk1_12->setStyleSheet("background-color: rgb(230, 230, 230)");

        for(i=0;i<96;i++)
        {
            DsMas[i]->setStyleSheet("background-color: rgb(230, 230, 230);");
            if (i<12)
             {
              dsAn[i]->setText("");
              AnIn[i]->setText("");
             }
            if (i <32)
             {
              DsOutMas[i]->setStyleSheet("background-color: rgb(230, 230, 230);");
             }
        }
        break;

    case 2:
        m_pForm->lab_txt_mk2_0->setText("");
        m_pForm->lab_txt_mk2_1->setText("");
        m_pForm->lab_txt_mk2_2->setText("");
        m_pForm->lab_txt_mk2_3->setText("");
        m_pForm->lab_txt_mk2_4->setText("");
        m_pForm->lab_txt_mk2_5->setText("");
        m_pForm->lab_txt_mk2_6->setText("");
        m_pForm->lab_txt_mk2_7->setText("");
        m_pForm->lab_txt_mk2_8->setText("");
        m_pForm->lab_txt_mk2_9->setText("");
        m_pForm->lab_txt_mk2_10->setText("");
        m_pForm->lab_txt_mk2_11->setText("");
        m_pForm->lab_txt_mk2_12->setText("");
        m_pForm->lab_txt_mk2_13->setText("");
        m_pForm->lab_txt_mk2_14->setText("");

        m_pForm->lImit_mk2->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb( 239, 239, 239)");
        //m_pForm->lDhcp_mk2->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb( 239, 239, 239)");
        m_pForm->lRs232_mk2->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb( 239, 239, 239)");

        m_pForm->lab_sig_mk2_0->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk2_1->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk2_2->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk2_3->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk2_4->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk2_5->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk2_6->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk2_7->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk2_8->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk2_9->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk2_10->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk2_11->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk2_12->setStyleSheet("background-color: rgb(230, 230, 230)");

        for(i=0;i<96;i++)
        {
            DsMas[96 + i]->setStyleSheet("background-color: rgb(230, 230, 230);");
            if (i>=12 && i<24)
             {
              dsAn[i]->setText("");
              AnIn[i]->setText("");
             }
            if (i>=32 && i <64)
             {
              DsOutMas[i]->setStyleSheet("background-color: rgb(230, 230, 230);");
             }
        }
        break;

    case 3:
        m_pForm->lab_txt_mk3_0->setText("");
        m_pForm->lab_txt_mk3_1->setText("");
        m_pForm->lab_txt_mk3_2->setText("");
        m_pForm->lab_txt_mk3_3->setText("");
        m_pForm->lab_txt_mk3_4->setText("");
        m_pForm->lab_txt_mk3_5->setText("");
        m_pForm->lab_txt_mk3_6->setText("");
        m_pForm->lab_txt_mk3_7->setText("");
        m_pForm->lab_txt_mk3_8->setText("");
        m_pForm->lab_txt_mk3_9->setText("");
        m_pForm->lab_txt_mk3_10->setText("");
        m_pForm->lab_txt_mk3_11->setText("");
        m_pForm->lab_txt_mk3_12->setText("");
        m_pForm->lab_txt_mk3_13->setText("");
        m_pForm->lab_txt_mk3_14->setText("");

        m_pForm->lImit_mk3->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb( 239, 239, 239)");
        //m_pForm->lDhcp_mk3->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb( 239, 239, 239)");
        m_pForm->lRs232_mk3->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb( 239, 239, 239)");

        m_pForm->lab_sig_mk3_0->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk3_1->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk3_2->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk3_3->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk3_4->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk3_5->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk3_6->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk3_7->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk3_8->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk3_9->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk3_10->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk3_11->setStyleSheet("background-color: rgb(230, 230, 230)");
        m_pForm->lab_sig_mk3_12->setStyleSheet("background-color: rgb(230, 230, 230)");

        for(i=0;i<96;i++)
        {
            DsMas[96 + 96 + i]->setStyleSheet("background-color: rgb(230, 230, 230);");
            if (i>=24 && i<36)
             {
              dsAn[i]->setText("");
              AnIn[i]->setText("");
             }
            if (i>=64 && i <96)
             {
              DsOutMas[i]->setStyleSheet("background-color: rgb(230, 230, 230);");
             }
        }
        break;

    case 0:
        break;

    default:
        break;
    }

}

//****************************************************************************************
void MainWidget::showData1()
{

#ifdef DEBUGSHOWDATA
    qDebug("   MainWidget ShowData1 start");
#endif
  if (m_pRecordStorage->isNew(1) && isRedrawingMainWindowData)
  {
  RecordBlackBox element = m_pRecordStorage->getLast(1);
  failStorage->chekFailRec(&element,NULL,NULL,NULL);

  m_pForm->lab_txt_mk1_0->setText(QString::number(element.Anlg[0],'g',4));
  m_pForm->lab_txt_mk1_1->setText(QString::number(element.Anlg[1],'g',4));
  m_pForm->lab_txt_mk1_2->setText(QString::number(element.Anlg[2],'g',4));
  m_pForm->lab_txt_mk1_3->setText(QString::number(element.Anlg[3],'g',4));
  m_pForm->lab_txt_mk1_4->setText(QString::number(element.Anlg[4],'g',4));
  m_pForm->lab_txt_mk1_5->setText(QString::number(element.Anlg[5],'g',4));
  m_pForm->lab_txt_mk1_6->setText(QString::number(element.Anlg[6],'g',4));
  m_pForm->lab_txt_mk1_7->setText(QString::number(element.Anlg[7],'g',4));
  m_pForm->lab_txt_mk1_8->setText(QString::number(element.Anlg[8],'g',4));
  m_pForm->lab_txt_mk1_9->setText(QString::number(element.Anlg[10],'g',4));
  m_pForm->lab_txt_mk1_10->setText(QString::number(element.Anlg[11],'g',4));
  m_pForm->lab_txt_mk1_11->setText(QString::number(element.Anlg[10] + element.Anlg[11],'g',4));
  m_pForm->lab_txt_mk1_12->setText(QString::number(element.Anlg[3] - element.Anlg[2],'g',4));
  m_pForm->lab_txt_mk1_13->setText(QString::number(element.Anlg[9],'g',4));
  m_pForm->lab_txt_mk1_14->setText(QString::number(element.Anlg[0] - element.Anlg[9],'g',4));

  if (m_pForm->groupBoxInpSIg->isVisible())
  {int bt=0;
   int i,j,k;

   for(i=0;i<20;i++)
   {

    if (i<4)
    {
        bt=element.OutCww1[i];
        for(j=0;j<8;j++)
          {k=j+i*8;
           if ((bt & 1)!=0)
           {
               DsOutMas[k]->setStyleSheet("background-color: rgb(0, 0, 255);");
           }else
           {
               DsOutMas[k]->setStyleSheet("background-color: rgb(230, 230, 230);");
           };

           bt=bt >> 1;
          }
    }

    if (i<12)
    {
     bt=element.InCww1[i];
     for(j=0;j<8;j++)
       {k=j+i*8;
        if ((bt & 1)!=0)
        {
            DsMas[k]->setStyleSheet("background-color: rgb(0, 0, 255);");
        }else
        {
            DsMas[k]->setStyleSheet("background-color: rgb(230, 230, 230);");
        };

        bt=bt >> 1;
       }

     } //i<12

     AnIn[0+i]->setText(QString::number(element.SgnAnl[0+i],'g',4));

     if (element.dsAn[i] != 0)
      {
         dsAn[i]->setStyleSheet("background-color: rgb(255, 0, 0);");
         dsAn[i]->setText( QString().sprintf("%x", element.dsAn[i]));

      }else
      {
         dsAn[i]->setStyleSheet("background-color: rgb(200, 255, 150);");
         dsAn[i]->setText("");
      }

   }

      // dN
      if ((element.OutCww1[0] & 1) != 0 )
      {
          m_pForm->lab_sig_mk1_8->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(0, 230, 0);");
      }else
      {
          m_pForm->lab_sig_mk1_8->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb(230, 0, 0);");
      }

      // dP2
      if ((element.OutCww1[0] & 2) != 0 )
      {
          m_pForm->lab_sig_mk1_9->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(0, 230, 0);");
      }else
      {
          m_pForm->lab_sig_mk1_9->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb(230, 0, 0);");
      }

      // dP1
      if ((element.OutCww1[0] & 4) != 0 )
      {
          m_pForm->lab_sig_mk1_10->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(0, 230, 0);");
      }else
      {
          m_pForm->lab_sig_mk1_10->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb(230, 0, 0);");
      }

      // dNтг
      if ((element.OutCww1[0] & 8) != 0 )
      {
          m_pForm->lab_sig_mk1_11->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(0, 230, 0);");
      }else
      {
          m_pForm->lab_sig_mk1_11->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb(230, 0, 0);");
      }

      // dTпв
      if ((element.OutCww1[0] & 16) != 0 )
      {
          m_pForm->lab_sig_mk1_12->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(0, 230, 0);");
      }else
      {
          m_pForm->lab_sig_mk1_12->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb(230, 0, 0);");
      }

      // Ispravnost'
      if ((element.OutCww3[0] & 1) != 0 )
      {
          m_pForm->lab_sig_mk1_0->setStyleSheet("color: rgb(0, 0, 0);background-color: rgb(0, 230, 0);");
          m_pForm->lab_sig_mk1_0->setText("Испр.");
      }else
      {
          m_pForm->lab_sig_mk1_0->setStyleSheet("color: rgb(255, 255, 255);background-color: rgb(230, 0, 0);");
          m_pForm->lab_sig_mk1_0->setText("Неисп.");
      }

      // V rabote
      if ((element.OutCww3[0] & 2) != 0 )
      {
          m_pForm->lab_sig_mk1_1->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(0, 230, 0);");
          m_pForm->lab_sig_mk1_1->setText("Регул.");
      }else
      {
          m_pForm->lab_sig_mk1_1->setStyleSheet("color: rgb(0, 0, 0);  background-color: rgb(230, 230, 230);");
          m_pForm->lab_sig_mk1_1->setText("Не в раб.");
      }

      // Komandy
      if ((element.OutCww3[0] & 12) == 0 )
      {
          m_pForm->lab_sig_mk1_3->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(230, 230, 230);");
          m_pForm->lab_sig_mk1_3->setText("Нет команд");
      }else
      {
          if ((element.OutCww3[0] & 4) != 0 )
          {
              m_pForm->lab_sig_mk1_3->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb( 255, 180, 90);");
              m_pForm->lab_sig_mk1_3->setText("Ком. Б");
          }else
          {
              m_pForm->lab_sig_mk1_3->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb( 255, 180, 90);");
              m_pForm->lab_sig_mk1_3->setText("Ком М");
          }
      }

      // Rezhim
      if ((element.OutCww3[0] & 0xF0) == 0 )
      {
          m_pForm->lab_sig_mk1_2->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(230, 230, 230);");
          m_pForm->lab_sig_mk1_2->setText("Нет режима");
      }else
      {
          if ((element.OutCww3[0] & 16) != 0 )
          {
              m_pForm->lab_sig_mk1_2->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb( 0, 230, 0);");
              m_pForm->lab_sig_mk1_2->setText("Режим Н");
          }else
          {
              if ((element.OutCww3[0] & 32) != 0 )
              {
                  if ((element.OutCww3[0] & 128) != 0 )
                  {
                      m_pForm->lab_sig_mk1_2->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb( 0, 230, 0);");
                      m_pForm->lab_sig_mk1_2->setText("Режим Топрч");
                  }else
                  {
                      m_pForm->lab_sig_mk1_2->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(0, 230, 0);");
                      m_pForm->lab_sig_mk1_2->setText("Режим Т");
                  }
              }else
              {
                  if ((element.OutCww3[0] & 64) != 0 )
                  {
                  m_pForm->lab_sig_mk1_2->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(0, 230, 0);");
                  m_pForm->lab_sig_mk1_2->setText("Режим Ср");
                  }
              }
          }
      }

      // Zapret B M
      if ((element.OutCww3[1] & 6) == 0 )
      {
          m_pForm->lab_sig_mk1_6->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(230, 230, 230);");
          m_pForm->lab_sig_mk1_6->setText("Нет запрета");
      }else
      {
          if ((element.OutCww3[1] & 2) != 0 )
          {
              m_pForm->lab_sig_mk1_6->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb( 230, 0, 0);");
              m_pForm->lab_sig_mk1_6->setText("Запрет Б");
          }else
          {
              m_pForm->lab_sig_mk1_6->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb( 230, 0, 0);");
              m_pForm->lab_sig_mk1_6->setText("Запрет М");
          }
      }

      // Zapret T
      if ((element.OutCww3[1] & 8) == 0 )
      {
          m_pForm->lab_sig_mk1_7->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(230, 230, 230);");
          m_pForm->lab_sig_mk1_7->setText("Нет запрета");
      }else
      {
          m_pForm->lab_sig_mk1_7->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(255, 180, 90);");
          m_pForm->lab_sig_mk1_7->setText("Запрет Т");
      }

      // Otklonenie N P2 B M
      if (((element.OutCww3[1] & 48) == 0 ) && ((element.OutCww3[1] & 192) == 0 ))
      {
          m_pForm->lab_sig_mk1_4->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(230, 230, 230)");
          m_pForm->lab_sig_mk1_4->setText("Нет откл");
      }else
      {
          if ((element.OutCww3[1] & 16) != 0 )
          {
              m_pForm->lab_sig_mk1_4->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb( 255, 180, 90);");
              m_pForm->lab_sig_mk1_4->setText("Откл_P2 'Б'");
          }

          if ((element.OutCww3[1] & 32) != 0 )
          {
              m_pForm->lab_sig_mk1_4->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb( 255, 180, 90);");
              m_pForm->lab_sig_mk1_4->setText("Откл_P2 'М'");
          }

          if ((element.OutCww3[1] & 64) != 0 )
          {
              m_pForm->lab_sig_mk1_4->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb( 255, 180, 90);");
              m_pForm->lab_sig_mk1_4->setText("Откл_N 'Б'");
          }
          if ((element.OutCww3[1] & 128) != 0 )
          {
             m_pForm->lab_sig_mk1_4->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb( 255, 180, 90);");
             m_pForm->lab_sig_mk1_4->setText("Откл_N 'М'");
          }
     }
  }

 if (element.OutCww1[0] & 0x20)
     m_pForm->lImit_mk1->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb( 255, 180, 90);");
 else
     m_pForm->lImit_mk1->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb( 239, 239, 239)");

 m_pForm->lRs232_mk1->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb( 0, 255, 0);");
 isNewMsg1 = true;
 }// if isNew

#ifdef DEBUGSHOWDATA
    qDebug("   MainWidget ShowData1 end");
#endif
}

//****************************************************************************************
void MainWidget::showData2()
{

#ifdef DEBUGSHOWDATA
    qDebug("   MainWidget ShowData2 start");
#endif
  if  (m_pRecordStorage->isNew(2) && isRedrawingMainWindowData)
  {
  RecordBlackBox element = m_pRecordStorage->getLast(2);
  failStorage->chekFailRec(NULL,&element,NULL,NULL);

  m_pForm->lab_txt_mk2_0->setText(QString::number(element.Anlg[0],'g',4));
  m_pForm->lab_txt_mk2_1->setText(QString::number(element.Anlg[1],'g',4));
  m_pForm->lab_txt_mk2_2->setText(QString::number(element.Anlg[2],'g',4));
  m_pForm->lab_txt_mk2_3->setText(QString::number(element.Anlg[3],'g',4));
  m_pForm->lab_txt_mk2_4->setText(QString::number(element.Anlg[4],'g',4));
  m_pForm->lab_txt_mk2_5->setText(QString::number(element.Anlg[5],'g',4));
  m_pForm->lab_txt_mk2_6->setText(QString::number(element.Anlg[6],'g',4));
  m_pForm->lab_txt_mk2_7->setText(QString::number(element.Anlg[7],'g',4));
  m_pForm->lab_txt_mk2_8->setText(QString::number(element.Anlg[8],'g',4));
  m_pForm->lab_txt_mk2_9->setText(QString::number(element.Anlg[10],'g',4));
  m_pForm->lab_txt_mk2_10->setText(QString::number(element.Anlg[11],'g',4));
  m_pForm->lab_txt_mk2_11->setText(QString::number(element.Anlg[10] + element.Anlg[11],'g',4));
  m_pForm->lab_txt_mk2_12->setText(QString::number(element.Anlg[3] - element.Anlg[2],'g',4));
  m_pForm->lab_txt_mk2_13->setText(QString::number(element.Anlg[9],'g',4));
  m_pForm->lab_txt_mk2_14->setText(QString::number(element.Anlg[0] - element.Anlg[9],'g',4));

  if (m_pForm->groupBoxInpSIg->isVisible())
  {int bt=0;
   int i,j,k;

       for(i=0;i<20;i++)
       {

         if (i<4)
         {
               bt=element.OutCww1[i];
               for(j=0;j<8;j++)
                 {k=j+i*8;
                  if ((bt & 1)!=0)
                  {
                      DsOutMas[k+32]->setStyleSheet("background-color: rgb(0, 0, 255);");
                  }else
                  {
                      DsOutMas[k+32]->setStyleSheet("background-color: rgb(230, 230, 230);");
                  };

                  bt=bt >> 1;
                 }
        }

        if (i<12)
        {
         bt=element.InCww1[i];
         for(j=0;j<8;j++)
           {k=j+i*8;
            if ((bt & 1)!=0)
            {
                DsMas[k+96]->setStyleSheet("background-color: rgb(0, 0, 255);");
            }else
            {
                DsMas[k+96]->setStyleSheet("background-color: rgb(230, 230, 230);");
            };

            bt=bt >> 1;
           }

        } //i<12

         AnIn[20+i]->setText(QString::number(element.SgnAnl[0+i],'g',4));

         if (element.dsAn[i] != 0)
          {
             dsAn[20+i]->setStyleSheet("background-color: rgb(255, 0, 0);");
             dsAn[20+i]->setText( QString().sprintf("%x", element.dsAn[i]));

          }else
          {
             dsAn[20+i]->setStyleSheet("background-color: rgb(200, 255, 150);");
             dsAn[20+i]->setText("");
          }

       }

      // dN
      if ((element.OutCww1[0] & 1) != 0 )
      {
          m_pForm->lab_sig_mk2_8->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(0, 230, 0);");
      }else
      {
          m_pForm->lab_sig_mk2_8->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb(230, 0, 0);");
      }

      // dP2
      if ((element.OutCww1[0] & 2) != 0 )
      {
          m_pForm->lab_sig_mk2_9->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(0, 230, 0);");
      }else
      {
          m_pForm->lab_sig_mk2_9->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb(230, 0, 0);");
      }

      // dP1
      if ((element.OutCww1[0] & 4) != 0 )
      {
          m_pForm->lab_sig_mk2_10->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(0, 230, 0);");
      }else
      {
          m_pForm->lab_sig_mk2_10->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb(230, 0, 0);");
      }

      // dNтг
      if ((element.OutCww1[0] & 8) != 0 )
      {
          m_pForm->lab_sig_mk2_11->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(0, 230, 0);");
      }else
      {
          m_pForm->lab_sig_mk2_11->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb(230, 0, 0);");
      }

      // dTпв
      if ((element.OutCww1[0] & 8) != 0 )
      {
          m_pForm->lab_sig_mk2_12->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(0, 230, 0);");
      }else
      {
          m_pForm->lab_sig_mk2_12->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb(230, 0, 0);");
      }

      // Ispravnost'
      if ((element.OutCww3[0] & 1) != 0 )
      {
          m_pForm->lab_sig_mk2_0->setStyleSheet("color: rgb(0, 0, 0);background-color: rgb(0, 230, 0);");
          m_pForm->lab_sig_mk2_0->setText("Испр.");
      }else
      {
          m_pForm->lab_sig_mk2_0->setStyleSheet("color: rgb(255, 255, 255);background-color: rgb(230, 0, 0);");
          m_pForm->lab_sig_mk2_0->setText("Неисп.");
      }

      // V rabote
      if ((element.OutCww3[0] & 2) != 0 )
      {
          m_pForm->lab_sig_mk2_1->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(0, 230, 0);");
          m_pForm->lab_sig_mk2_1->setText("Регул.");
      }else
      {
          m_pForm->lab_sig_mk2_1->setStyleSheet("color: rgb(0, 0, 0);  background-color: rgb(230, 230, 230);");
          m_pForm->lab_sig_mk2_1->setText("Не в раб.");
      }

      // Komandy
      if ((element.OutCww3[0] & 12) == 0 )
      {
          m_pForm->lab_sig_mk2_3->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(230, 230, 230);");
          m_pForm->lab_sig_mk2_3->setText("Нет команд");
      }else
      {
          if ((element.OutCww3[0] & 4) != 0 )
          {
              m_pForm->lab_sig_mk2_3->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb( 255, 180, 90);");
              m_pForm->lab_sig_mk2_3->setText("Ком. Б");
          }else
          {
              m_pForm->lab_sig_mk2_3->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb( 255, 180, 90);");
              m_pForm->lab_sig_mk2_3->setText("Ком М");
          }
      }

      // Rezhim
      if ((element.OutCww3[0] & 0xF0) == 0 )
      {
          m_pForm->lab_sig_mk2_2->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(230, 230, 230);");
          m_pForm->lab_sig_mk2_2->setText("Нет режима");
      }else
      {
          if ((element.OutCww3[0] & 16) != 0 )
          {
              m_pForm->lab_sig_mk2_2->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb( 0, 230, 0);");
              m_pForm->lab_sig_mk2_2->setText("Режим Н");
          }else
          {
              if ((element.OutCww3[0] & 32) != 0 )
              {
                  if ((element.OutCww3[0] & 128) != 0 )
                  {
                      m_pForm->lab_sig_mk2_2->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb( 0, 230, 0);");
                      m_pForm->lab_sig_mk2_2->setText("Режим Топрч");
                  }else
                  {
                          m_pForm->lab_sig_mk2_2->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(0, 230, 0);");
                          m_pForm->lab_sig_mk2_2->setText("Режим Т");
                  }
              }else
              {
                  if ((element.OutCww3[0] & 64) != 0 )
                  {
                  m_pForm->lab_sig_mk2_2->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(0, 230, 0);");
                  m_pForm->lab_sig_mk2_2->setText("Режим Ср");
                  }
              }
          }
      }

      // Zapret B M
      if ((element.OutCww3[1] & 6) == 0 )
      {
          m_pForm->lab_sig_mk2_6->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(230, 230, 230);");
          m_pForm->lab_sig_mk2_6->setText("Нет запрета");
      }else
      {
          if ((element.OutCww3[1] & 2) != 0 )
          {
              m_pForm->lab_sig_mk2_6->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb( 230, 0, 0);");
              m_pForm->lab_sig_mk2_6->setText("Запрет Б");
          }else
          {
              m_pForm->lab_sig_mk2_6->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb( 230, 0, 0);");
              m_pForm->lab_sig_mk2_6->setText("Запрет М");
          }
      }

      // Zapret T
      if ((element.OutCww3[1] & 8) == 0 )
      {
          m_pForm->lab_sig_mk2_7->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(230, 230, 230);");
          m_pForm->lab_sig_mk2_7->setText("Нет запрета");
      }else
      {
          m_pForm->lab_sig_mk2_7->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(255, 180, 90);");
          m_pForm->lab_sig_mk2_7->setText("Запрет Т");
      }

      // Otklonenie N P2 B M
      if (((element.OutCww3[1] & 48) == 0 ) && ((element.OutCww3[1] & 192) == 0 ))
      {
          m_pForm->lab_sig_mk2_4->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(230, 230, 230)");
          m_pForm->lab_sig_mk2_4->setText("Нет откл");
      }else
      {
          if ((element.OutCww3[1] & 16) != 0 )
          {
              m_pForm->lab_sig_mk2_4->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb( 255, 180, 90);");
              m_pForm->lab_sig_mk2_4->setText("Откл_P2 'Б'");
          }

          if ((element.OutCww3[1] & 32) != 0 )
          {
              m_pForm->lab_sig_mk2_4->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb( 255, 180, 90);");
              m_pForm->lab_sig_mk2_4->setText("Откл_P2 'М'");
          }

          if ((element.OutCww3[1] & 64) != 0 )
          {
              m_pForm->lab_sig_mk2_4->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb( 255, 180, 90);");
              m_pForm->lab_sig_mk2_4->setText("Откл_N 'Б'");
          }
          if ((element.OutCww3[1] & 128) != 0 )
          {
             m_pForm->lab_sig_mk2_4->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb( 255, 180, 90);");
             m_pForm->lab_sig_mk2_4->setText("Откл_N 'М'");
          }
      }
  }

  if (element.OutCww1[0] & 0x20)
      m_pForm->lImit_mk2->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb( 255, 180, 90);");
  else
      m_pForm->lImit_mk2->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb( 239, 239, 239)");

  m_pForm->lRs232_mk2->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb( 0, 255, 0);");
  isNewMsg2 = true;
  }//if isNew()

#ifdef DEBUGSHOWDATA
    qDebug("   MainWidget ShowData2 end");
#endif
}

//****************************************************************************************
void MainWidget::showData3()
{

#ifdef DEBUGSHOWDATA
    qDebug("   MainWidget ShowData3 start");
#endif
  if (m_pRecordStorage->isNew(3) && isRedrawingMainWindowData)
  {
  RecordBlackBox element = m_pRecordStorage->getLast(3);
  failStorage->chekFailRec(NULL,NULL,&element,NULL);

  m_pForm->lab_txt_mk3_0->setText(QString::number(element.Anlg[0],'g',4));
  m_pForm->lab_txt_mk3_1->setText(QString::number(element.Anlg[1],'g',4));
  m_pForm->lab_txt_mk3_2->setText(QString::number(element.Anlg[2],'g',4));
  m_pForm->lab_txt_mk3_3->setText(QString::number(element.Anlg[3],'g',4));
  m_pForm->lab_txt_mk3_4->setText(QString::number(element.Anlg[4],'g',4));
  m_pForm->lab_txt_mk3_5->setText(QString::number(element.Anlg[5],'g',4));
  m_pForm->lab_txt_mk3_6->setText(QString::number(element.Anlg[6],'g',4));
  m_pForm->lab_txt_mk3_7->setText(QString::number(element.Anlg[7],'g',4));
  m_pForm->lab_txt_mk3_8->setText(QString::number(element.Anlg[8],'g',4));
  m_pForm->lab_txt_mk3_9->setText(QString::number(element.Anlg[10],'g',4));
  m_pForm->lab_txt_mk3_10->setText(QString::number(element.Anlg[11],'g',4));
  m_pForm->lab_txt_mk3_11->setText(QString::number(element.Anlg[10] + element.Anlg[11],'g',4));
  m_pForm->lab_txt_mk3_12->setText(QString::number(element.Anlg[3] - element.Anlg[2],'g',4));
  m_pForm->lab_txt_mk3_13->setText(QString::number(element.Anlg[9],'g',4));
  m_pForm->lab_txt_mk3_14->setText(QString::number(element.Anlg[0] - element.Anlg[9],'g',4));

  if (m_pForm->groupBoxInpSIg->isVisible() )
  {int bt=0;
   int i,j,k;

       for(i=0;i<20;i++)
       {

       if (i<4)
       {
         bt=element.OutCww1[i];
         for(j=0;j<8;j++)
         {k=j+i*8;
          if ((bt & 1)!=0)
          {
           DsOutMas[k+64]->setStyleSheet("background-color: rgb(0, 0, 255);");
          }else
          {
           DsOutMas[k+64]->setStyleSheet("background-color: rgb(230, 230, 230);");
          };
          bt=bt >> 1;
         }
       }

        if (i<12)
        {
         if (i<4)
         {
             bt=element.InCww1[i];
         }else  if (i<8)
         {
             bt=element.InCww2[i-4];
         }else
         {
             bt=element.InCww3[i-8];
         }

         for(j=0;j<8;j++)
           {k=j+i*8;
            if ((bt & 1)!=0)
            {
                DsMas[k+96+96]->setStyleSheet("background-color: rgb(0, 0, 255);");
            }else
            {
                DsMas[k+96+96]->setStyleSheet("background-color: rgb(230, 230, 230);");
            };

            bt=bt >> 1;
           }

        } //i<12

        AnIn[40+i]->setText(QString::number(element.SgnAnl[0+i],'g',4));

        if (element.dsAn[i] != 0)
         {
            dsAn[40+i]->setStyleSheet("background-color: rgb(255, 0, 0);");
            dsAn[40+i]->setText( QString().sprintf("%x", element.dsAn[i]));

         }else
         {
            dsAn[40+i]->setStyleSheet("background-color: rgb(200, 255, 150);");
            dsAn[40+i]->setText("");
         }

       }

      // dN
      if ((element.OutCww1[0] & 1) != 0 )
      {
          m_pForm->lab_sig_mk3_8->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(0, 230, 0);");
      }else
      {
          m_pForm->lab_sig_mk3_8->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb(230, 0, 0);");
      }

      // dP2
      if ((element.OutCww1[0] & 2) != 0 )
      {
          m_pForm->lab_sig_mk3_9->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(0, 230, 0);");
      }else
      {
          m_pForm->lab_sig_mk3_9->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb(230, 0, 0);");
      }

      // dP1
      if ((element.OutCww1[0] & 4) != 0 )
      {
          m_pForm->lab_sig_mk3_10->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(0, 230, 0);");
      }else
      {
          m_pForm->lab_sig_mk3_10->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb(230, 0, 0);");
      }

      // dNтг
      if ((element.OutCww1[0] & 8) != 0 )
      {
          m_pForm->lab_sig_mk3_11->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(0, 230, 0);");
      }else
      {
          m_pForm->lab_sig_mk3_11->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb(230, 0, 0);");
      }

      // dTпв
      if ((element.OutCww1[0] & 8) != 0 )
      {
          m_pForm->lab_sig_mk3_12->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(0, 230, 0);");
      }else
      {
          m_pForm->lab_sig_mk3_12->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb(230, 0, 0);");
      }

      // Ispravnost'
      if ((element.OutCww3[0] & 1) != 0 )
      {
          m_pForm->lab_sig_mk3_0->setStyleSheet("color: rgb(0, 0, 0);background-color: rgb(0, 230, 0);");
          m_pForm->lab_sig_mk3_0->setText("Испр.");
      }else
      {
          m_pForm->lab_sig_mk3_0->setStyleSheet("color: rgb(255, 255, 255);background-color: rgb(230, 0, 0);");
          m_pForm->lab_sig_mk3_0->setText("Неисп.");
      }

      // V rabote
      if ((element.OutCww3[0] & 2) != 0 )
      {
          m_pForm->lab_sig_mk3_1->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(0, 230, 0);");
          m_pForm->lab_sig_mk3_1->setText("Регул.");
      }else
      {
          m_pForm->lab_sig_mk3_1->setStyleSheet("color: rgb(0, 0, 0);  background-color: rgb(230, 230, 230);");
          m_pForm->lab_sig_mk3_1->setText("Не в раб.");
      }

      // Komandy
      if ((element.OutCww3[0] & 12) == 0 )
      {
          m_pForm->lab_sig_mk3_3->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(230, 230, 230);");
          m_pForm->lab_sig_mk3_3->setText("Нет команд");
      }else
      {
          if ((element.OutCww3[0] & 4) != 0 )
          {
              m_pForm->lab_sig_mk3_3->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb( 255, 180, 90);");
              m_pForm->lab_sig_mk3_3->setText("Ком. Б");
          }else
          {
              m_pForm->lab_sig_mk3_3->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb( 255, 180, 90);");
              m_pForm->lab_sig_mk3_3->setText("Ком М");
          }
      }

      // Rezhim
      if ((element.OutCww3[0] & 0xF0) == 0 )
      {
          m_pForm->lab_sig_mk3_2->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(230, 230, 230);");
          m_pForm->lab_sig_mk3_2->setText("Нет режима");
      }else
      {
          if ((element.OutCww3[0] & 16) != 0 )
          {
              m_pForm->lab_sig_mk3_2->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb( 0, 230, 0);");
              m_pForm->lab_sig_mk3_2->setText("Режим Н");
          }else
          {
              if ((element.OutCww3[0] & 32) != 0 )
              {
                  if ((element.OutCww3[0] & 128) != 0 )
                  {
                      m_pForm->lab_sig_mk3_2->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb( 0, 230, 0);");
                      m_pForm->lab_sig_mk3_2->setText("Режим Топрч");
                  }else
                  {
                      m_pForm->lab_sig_mk3_2->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(0, 230, 0);");
                      m_pForm->lab_sig_mk3_2->setText("Режим Т");
                  }
              }else
              {
                  if ((element.OutCww3[0] & 64) != 0 )
                  {
                  m_pForm->lab_sig_mk3_2->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(0, 230, 0);");
                  m_pForm->lab_sig_mk3_2->setText("Режим Ср");
                  }
              }
          }
      }

      // Zapret B M
      if ((element.OutCww3[1] & 6) == 0 )
      {
          m_pForm->lab_sig_mk3_6->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(230, 230, 230);");
          m_pForm->lab_sig_mk3_6->setText("Нет запрета");
      }else
      {
          if ((element.OutCww3[1] & 2) != 0 )
          {
              m_pForm->lab_sig_mk3_6->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb( 230, 0, 0);");
              m_pForm->lab_sig_mk3_6->setText("Запрет Б");
          }else
          {
              m_pForm->lab_sig_mk3_6->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb( 230, 0, 0);");
              m_pForm->lab_sig_mk3_6->setText("Запрет М");
          }
      }

      // Zapret T
      if ((element.OutCww3[1] & 8) == 0 )
      {
          m_pForm->lab_sig_mk3_7->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(230, 230, 230);");
          m_pForm->lab_sig_mk3_7->setText("Нет запрета");
      }else
      {
          m_pForm->lab_sig_mk3_7->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(255, 180, 90);");
          m_pForm->lab_sig_mk3_7->setText("Запрет Т");
      }

      // Otklonenie N P2 B M
      if (((element.OutCww3[1] & 48) == 0 ) && ((element.OutCww3[1] & 192) == 0 ))
      {
          m_pForm->lab_sig_mk3_4->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb(230, 230, 230)");
          m_pForm->lab_sig_mk3_4->setText("Нет откл");
      }else
      {
          if ((element.OutCww3[1] & 16) != 0 )
          {
              m_pForm->lab_sig_mk3_4->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb( 255, 180, 90);");
              m_pForm->lab_sig_mk3_4->setText("Откл_P2 'Б'");
          }

          if ((element.OutCww3[1] & 32) != 0 )
          {
              m_pForm->lab_sig_mk3_4->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb( 255, 180, 90);");
              m_pForm->lab_sig_mk3_4->setText("Откл_P2 'М'");
          }


          if ((element.OutCww3[1] & 64) != 0 )
          {
              m_pForm->lab_sig_mk3_4->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb( 255, 180, 90);");
              m_pForm->lab_sig_mk3_4->setText("Откл_N 'Б'");
          }
          if ((element.OutCww3[1] & 128) != 0 )
          {
             m_pForm->lab_sig_mk3_4->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb( 255, 180, 90);");
             m_pForm->lab_sig_mk3_4->setText("Откл_N 'М'");
          }
      }
  }

  if (element.OutCww1[0] & 0x20)
      m_pForm->lImit_mk3->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb( 255, 180, 90);");
  else
      m_pForm->lImit_mk3->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb( 239, 239, 239)");

 m_pForm->lRs232_mk3->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb( 0, 255, 0);");
 isNewMsg3 = true;
 }//if isNew()
#ifdef DEBUGSHOWDATA
    qDebug("   MainWidget ShowData3 end");
#endif
}

//****************************************************************************************
void MainWidget::showData4()
{

#ifdef DEBUGSHOWDATA
    qDebug("   MainWidget ShowData4 start");
#endif
  if (m_pRecordStorage->isNew(4) && isRedrawingMainWindowData)
  {
  m_pForm->lRs232_mk4->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb( 0, 255, 0);");
  isNewMsg4 = true;
  }//if isNew()
#ifdef DEBUGSHOWDATA
    qDebug("   MainWidget ShowData4 end");
#endif
}

//****************************************************************************************
void MainWidget::on_new_element_mk1(RecordBlackBox element)
{
#ifdef DEBUGSHOWDATA
    qDebug("   MainWidget onnewelement1 ");
#endif

    if (timeRecArchive1 != QDateTime::currentDateTime().time().hour())
    {
        m_pRecordStorage->saveToFile(1);
        timeRecArchive1 = QDateTime::currentDateTime().time().hour();
    }

    m_pRecordStorage->add(element,1);
    showData1();
}

//****************************************************************************************
void MainWidget::on_new_element_mk2(RecordBlackBox element)
{
#ifdef DEBUGSHOWDATA
    qDebug("   MainWidget onnewelement2 ");
#endif

    if (timeRecArchive2 != QDateTime::currentDateTime().time().hour())
    {
        m_pRecordStorage->saveToFile(2);
        timeRecArchive2 = QDateTime::currentDateTime().time().hour();
    }

    m_pRecordStorage->add(element,2);
   showData2();
}

//****************************************************************************************
void MainWidget::on_new_element_mk3(RecordBlackBox element)
{
#ifdef DEBUGSHOWDATA
    qDebug("   MainWidget onnewelement3 ");
#endif

    if (timeRecArchive3 != QDateTime::currentDateTime().time().hour())
    {
        m_pRecordStorage->saveToFile(3);
        timeRecArchive3 = QDateTime::currentDateTime().time().hour();
    }

    m_pRecordStorage->add(element,3);
   showData3();
}

//****************************************************************************************
void MainWidget::on_new_element_mk4(RecordBlackBox element)
{
    bool failedDoor=false;
    bool failedSuply=false;
    quint8 i;
    quint8 j;
    quint8 byt;
    quint16 l;
    quint16 k;
    RecordBlackBox elem;
#ifdef DEBUGSHOWDATA
    qDebug("   MainWidget onnewelement3 ");
#endif

    if (timeRecArchive4 != QDateTime::currentDateTime().time().hour())
    {
        m_pRecordStorage->saveToFile(4);
        timeRecArchive4 = QDateTime::currentDateTime().time().hour();
    }

    if (avtWidget)
    if (avtWidget->en)
    {
        avtWidget->screenRefresh(element);
    }else
    {
        avtWidget =0;
    }


failStorage->chekFailRec(NULL,NULL,NULL,&element);

m_pForm->lRs232_mk4->setStyleSheet("color: rgb(0, 0, 0); background-color: rgb( 0, 255, 0);");
m_pRecordStorage->add(element,4);

   showData4();
}

/*void MainWidget::on_pushButton_clicked()
{

   m_pRecordStorage->saveToFile();

}*/

//****************************************************************************************
void MainWidget::updateTimer500mls()
{
    static qint8 countOneSecond=0;
    static qint8 countHalfMinute=0;
    static quint16 countHalfHour=0;

    static qint8 counNewMsg1=0;
    static qint8 counNewMsg2=0;
    static qint8 counNewMsg3=0;
    static qint8 counNewMsg4=0;
    qint8 hh;
    qint8 mm;
    static bool isDots=false;
    static QString timeString1,timeString2;

       if (!isNewMsg1) counNewMsg1++;else counNewMsg1=0;
       if (!isNewMsg2) counNewMsg2++;else counNewMsg2=0;
       if (!isNewMsg3) counNewMsg3++;else counNewMsg3=0;
       if (!isNewMsg4) counNewMsg4++;else counNewMsg4=0;

       if (counNewMsg1>=20)
        {
           clearGraphWindow(1); counNewMsg1 =0;
           if (!wasDisconnectMK1)
           {
               failStorage->saveToFileOneString(false, QDate::currentDate().toString("dd/MM/yy").leftJustified(10,'_') + Util::timeCharToString((char *)(m_pRecordStorage->getLast(1).time)).leftJustified(15,'_') + "MK1___установка___________ПМИ07Р______________нет ответа" +  "\r\n" +   "\r\n");
               wasDisconnectMK1 = true;
           }
        }else
        {
           if (wasDisconnectMK1 && isNewMsg1)
           {
            failStorage->saveToFileOneString(false, QDate::currentDate().toString("dd/MM/yy").leftJustified(10,'_') + Util::timeCharToString((char *)(m_pRecordStorage->getLast(1).time)).leftJustified(15,'_') + "MK1___снятие________________ПМИ07Р______________нет ответа" +   "\r\n" +   "\r\n");
            wasDisconnectMK1 = false;
           }
        }

       if (counNewMsg2>=20)
        {
           clearGraphWindow(2); counNewMsg2 =0;
           if (!wasDisconnectMK2)
           {
               failStorage->saveToFileOneString(false, QDate::currentDate().toString("dd/MM/yy").leftJustified(10,'_') + Util::timeCharToString((char *)(m_pRecordStorage->getLast(2).time)).leftJustified(15,'_') + "MK2___установка___________ПМИ07Р______________нет ответа" +   '\r' +'\n'+   '\r' +'\n');
               wasDisconnectMK2 = true;
           }
        }else
        {
           if (wasDisconnectMK2 && isNewMsg2)
           {
            failStorage->saveToFileOneString(false, QDate::currentDate().toString("dd/MM/yy").leftJustified(10,'_') + Util::timeCharToString((char *)(m_pRecordStorage->getLast(2).time)).leftJustified(15,'_') + "MK2___снятие________________ПМИ07Р______________нет ответа" +   '\r' +'\n'+   '\r' +'\n');
            wasDisconnectMK2 = false;
           }
        }

       if (counNewMsg3>=20)
        {
           clearGraphWindow(3); counNewMsg3 =0;
           if (!wasDisconnectMK3)
           {
               failStorage->saveToFileOneString(false, QDate::currentDate().toString("dd/MM/yy").leftJustified(10,'_') + Util::timeCharToString((char *)(m_pRecordStorage->getLast(3).time)).leftJustified(15,'_') + "MK3___установка___________ПМИ07Р______________нет ответа" +  "\r\n"+  "\r\n");
               wasDisconnectMK3 = true;
           }
        }else
        {
           if (wasDisconnectMK3 && isNewMsg3)
           {
            failStorage->saveToFileOneString(false, QDate::currentDate().toString("dd/MM/yy").leftJustified(10,'_') + Util::timeCharToString((char *)(m_pRecordStorage->getLast(3).time)).leftJustified(15,'_') + "MK3___снятие________________ПМИ07Р______________нет ответа" +  "\r\n"+  "\r\n");
            wasDisconnectMK3 = false;
           }
        }

       if (counNewMsg4>=20)
        {
           clearGraphWindow(4); counNewMsg4 =0;
           if (!wasDisconnectMK4)
           {
               failStorage->saveToFileOneString(false, QDate::currentDate().toString("dd/MM/yy").leftJustified(10,'_') + Util::timeCharToString((char *)(m_pRecordStorage->getLast(4).time)).leftJustified(15,'_') + "MK4___установка___________ПМИ07Р______________нет ответа" +  "\r\n"+  "\r\n");
               wasDisconnectMK4 = true;
           }
        }else
        {
           if (wasDisconnectMK4 && isNewMsg4)
           {
            failStorage->saveToFileOneString(false, QDate::currentDate().toString("dd/MM/yy").leftJustified(10,'_') + Util::timeCharToString((char *)(m_pRecordStorage->getLast(4).time)).leftJustified(15,'_') + "MK4___снятие________________ПМИ07Р______________нет ответа" +  "\r\n"+  "\r\n");
            wasDisconnectMK4 = false;
           }
        }
      /* if (counNewMsg2>=20) {clearGraphWindow(2); counNewMsg2 =0; }
       if (counNewMsg3>=20) {clearGraphWindow(3); counNewMsg3 =0; }
       if (counNewMsg4>=20) {clearGraphWindow(4); counNewMsg4 =0; }*/

       m_pForm->lRs232_mk1->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb( 239, 239, 239);");
       m_pForm->lRs232_mk2->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb( 239, 239, 239);");
       m_pForm->lRs232_mk3->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb( 239, 239, 239);");
       m_pForm->lRs232_mk4->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb( 239, 239, 239);");

       isNewMsg1 = false;
       isNewMsg2 = false;
       isNewMsg3 = false;
       isNewMsg4 = false;

       if (countOneSecond<=0)
       {
           if ( isDots)
           {
             m_pForm->labelTimeDate->setText(timeString1);
           }else
           {
             m_pForm->labelTimeDate->setText(timeString2);
           }

           isDots = !isDots;
           countOneSecond=2;
       }

       if (countHalfMinute<=0)
       {
           timeString1 = QDate::currentDate().toString("dd/MM/yyyy | ")+QTime::currentTime().toString("HH mm");
           timeString2 = QDate::currentDate().toString("dd/MM/yyyy | ")+QTime::currentTime().toString("HH:mm");

           countHalfMinute = 60;

           hh= QDateTime::currentDateTime().time().hour();
           mm =  QDateTime::currentDateTime().time().minute();

           if (hh==0)
           {
#ifdef DEBUGALL
    qDebug() << "updateTimer500mls hh=0" << archIsRared << archIsSaved;
#endif
               archIsRared =false;
               archIsSaved =false;
           }

           if (hh==23 && mm==55 && !archIsSaved)
             {

#ifdef DEBUGALL
    qDebug() << "updateTimer500mls hh==23 mm==55 START"<< archIsRared << archIsSaved;
#endif
             m_pRecordStorage->saveToFile(1);
             m_pRecordStorage->saveToFile(2);
             m_pRecordStorage->saveToFile(3);
             m_pRecordStorage->saveToFile(4);
#ifdef DEBUGALL
    qDebug() << "updateTimer500mls hh==23 mm==55 STOP"<< archIsRared << archIsSaved;
#endif
              archIsSaved = true;
             }

           if (hh==23 && mm==58 && !archIsRared)
             {
#ifdef DEBUGALL
    qDebug() << "updateTimer500mls hh==23 mm==58 START"<< archIsRared << archIsSaved;
#endif
            /* m_pRecordStorage->saveToFile(1);
             m_pRecordStorage->saveToFile(2);
             m_pRecordStorage->saveToFile(3);
             m_pRecordStorage->saveToFile(4);*/

             archiveBase();

#ifdef DEBUGALL
    qDebug() << "updateTimer500mls hh==23 mm==58 STOP"<< archIsRared << archIsSaved;
#endif
             archIsRared = true;
             }



           if (needSendTime>0)
           {// если еще нужно делать посылки времени - делаем каждые полминуты пока needSendTime не ноль
#ifdef DEBUGALL
    qDebug() << "needSendTime START";
#endif
               sendRS232Time();
               needSendTime--;
#ifdef DEBUGALL
    qDebug() << "needSendTime STOP";
#endif
           }

       }


       if (countHalfHour<=0)
       {
#ifdef DEBUGALL
    qDebug() << "cleanArchives START";
#endif
           countHalfHour = 3600;

           cleanArchives();
#ifdef DEBUGALL
    qDebug() << "cleanArchives STOP";
#endif
           needSendTime = 3;  // каждые полчаса делать по три посылки времени
       }

       countOneSecond--;
       countHalfMinute--;
       countHalfHour--;

}

//****************************************************************************************
void MainWidget::sendRS232Time()
{

#ifdef DEBUGSHOWDATA
    qDebug("   MainWidget sendRS232Time");
#endif

    quint8 txMsg[6];
    char ptr[3]={5,33,127};
    QString *strTreminal = new QString(ptr);

    QString *timeString = new QString();

    QDate qdate= QDate::currentDate();
    QTime qtime=QTime::currentTime();
    quint16 summa=0;
    quint8 ll=0;

    txMsg[0] =  qdate.year()-2000;
    txMsg[1] =  qdate.month();
    txMsg[2] =  qdate.day();
    txMsg[3] =  qtime.hour();
    txMsg[4] =  qtime.minute();
    txMsg[5] =  qtime.second();

    *timeString = "time" + Util::Trans1In2(txMsg[0]) + Util::Trans1In2(txMsg[1]) + Util::Trans1In2(txMsg[2]) + Util::Trans1In2(txMsg[3]) + Util::Trans1In2(txMsg[4]) + Util::Trans1In2(txMsg[5]);
    ll=timeString->length();
    summa = Util::scKS(timeString,ll);

    *strTreminal += Util::Trans2In4(summa) + Util::Trans1In2(ll) + *timeString;

    m_pSock1->sendString(strTreminal);
    m_pSock2->sendString(strTreminal);
    m_pSock3->sendString(strTreminal);
    m_pSock4->sendString(strTreminal);

   /* m_pForm->txtMK1->append("time sent" + QString::number( qtime.hour()) + ":"+ QString::number( qtime.minute()) + ":"+QString::number( qtime.second()) + "\r\n");
    m_pForm->txtMK2->append("time sent" + QString::number( qtime.hour()) + ":"+ QString::number( qtime.minute()) + ":"+QString::number( qtime.second()) + "\r\n");
    m_pForm->txtMK3->append("time sent" + QString::number( qtime.hour()) + ":"+ QString::number( qtime.minute()) + ":"+QString::number( qtime.second()) + "\r\n");
    m_pForm->txtMK4->append("time sent" + QString::number( qtime.hour()) + ":"+ QString::number( qtime.minute()) + ":"+QString::number( qtime.second()) + "\r\n");*/

}

/*void MainWidget::slotShowCommonJourWidget()
{

    jourWidget = new JourWindow(this,1,failStorage);
    jourWidget->show();
}

void MainWidget::slotShowLocalJourWidget()
{

    jourWidget = new JourWindow(this,2,failStorage);
    jourWidget->show();
}
*/

//****************************************************************************************
void MainWidget::slotShowSpaceMess()
{
 QMessageBox::critical(this,"внимание", "Заканчивается место под архивы. В ближаешее время архивы будут очищены автоматически. При необходимости сохраните нужные!") ;

}

//****************************************************************************************
void MainWidget::slotShowAvtWidget()
{
    avtWidget = new AvtWindow(this);
    avtWidget->show();
}


//****************************************************************************************
void MainWidget::slotShowArchive()
{
    showGraphWindow(0);
}

//****************************************************************************************
void MainWidget::slotShowGraphicCh1()
{
    showGraphWindow(1);

}

//****************************************************************************************
void MainWidget::slotShowGraphicCh2()
{
    showGraphWindow(2);

}

//****************************************************************************************
void MainWidget::slotShowGraphicCh3()
{
    showGraphWindow(3);
}

//****************************************************************************************
void MainWidget::setRedrawingMainWindowData(bool flag)
{
    isRedrawingMainWindowData = flag;
}

//****************************************************************************************
void MainWidget::showGraphWindow(qint8 chan)
{

}

//****************************************************************************************
void MainWidget::slotExit()
{
    passAction =1;
    m_pForm->framePassword->setVisible(true);
}

//****************************************************************************************
void MainWidget::slotSendOnOut()
{
#ifdef DEBUGSHOWDATA
    qDebug("   MainWidget slotSendOut ");
#endif
    char ptr[14]={5,33,127,'0','1','8','9','0','4','o','u','t','1',13};
    QString *strTreminal = new QString(ptr);

    m_pSock1->sendString(strTreminal);
    m_pSock2->sendString(strTreminal);
    m_pSock3->sendString(strTreminal);
    m_pSock4->sendString(strTreminal);
}

//****************************************************************************************
void MainWidget::slotSaveToFlash()
{

  QDir dirObj(homeDir +"/archives");
  //QFile file;
  QString destDirName;
  QFileInfo fileInfo;
  QDateTime createDate;

 archiveBase();

  passAction =1;
  m_pForm->framePassword->setVisible(true);

  //cleanArchives();

 // QDateTime setDate;


    //si = dirObj.Size;

 /* qint8 ret;
  qint8 rrr;


  QString NTPSERVERADDRESS="192.168.150.254";
  QString com="sudo /usr/sbin/ntpdate " + NTPSERVERADDRESS;
  ret = system(com.toAscii().constData());
*/



//  system("musb.sh -m");
 // this->slotExit();

          /* destDirName=QFileDialog::getExistingDirectory(this,"Выберите директорию для сохранения","/home/flex/usb",0);



  foreach(QString fileName,dirObj.entryList())
  {
      if (fileName!="." && fileName!="..")
      {

     fileInfo.setFile(fileName);
      createDate = fileInfo.lastModified();

//      if (createDate.date().day() == setDate.date().day())
      {

         QFile::copy(dirObj.absoluteFilePath(fileName),destDirName + "/"+fileName);
       //  dirObj.remove(fileName);
      //}

      }


  }




*/
}

//****************************************************************************************
void MainWidget::slotPassButt0()
{
    enterPassword +="1";
}

//****************************************************************************************
void MainWidget::slotPassButt1()
{
    enterPassword +="2";
}

//****************************************************************************************
void MainWidget::slotPassButt2()
{
    enterPassword +="3";
}

//****************************************************************************************
void MainWidget::slotPassButt3()
{
    enterPassword +="4";
}

//****************************************************************************************
void MainWidget::slotPassButt4()
{
    enterPassword +="5";
}

//****************************************************************************************
void MainWidget::slotPassButt5()
{
    enterPassword +="6";
}

//****************************************************************************************
void MainWidget::slotPassButt6()
{
    enterPassword +="7";
}

//****************************************************************************************
void MainWidget::slotPassButt7()
{
    enterPassword +="8";
}

//****************************************************************************************
void MainWidget::slotPassButt8()
{
    enterPassword +="9";
}

//****************************************************************************************
void MainWidget::slotPassButt9()
{
    enterPassword +="0";
}

//****************************************************************************************
void MainWidget::slotPassButt10()
{
    enterPassword ="";
    m_pForm->framePassword->setVisible(false);
}

//****************************************************************************************
void MainWidget::slotPassButt11()
{
    if (enterPassword==ADMINPASS)
    {
        switch (passAction) {
        case 1:
            passAction =0;
            EXIT();

            break;
        default:
            break;
        }
    }
    enterPassword = "";
    m_pForm->framePassword->setVisible(false);
}

//****************************************************************************************
void MainWidget::EXIT()
{

    m_pRecordStorage->saveToFile(1);
    m_pRecordStorage->saveToFile(2);
    m_pRecordStorage->saveToFile(3);
    m_pRecordStorage->saveToFile(4);

    system("killall restartPult.sh");
    this->close();
}

#ifdef DEBUGALL

//****************************************************************************************
void messageHandler(QtMsgType type,const char *msg)
{
     QTextStream out(m_logFile.data());
     out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ");
     out << msg << endl;
     out.flush();
}
#endif
