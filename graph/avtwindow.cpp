#include "avtwindow.h"
#include "ui_avtwindow.h"
#include "mainwidget.h"
#include <QCloseEvent>

//****************************************************************************************
AvtWindow::AvtWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AvtWindow)
{
    ui->setupUi(this);
  qobject_cast<MainWidget *>(this->parent())->setRedrawingMainWindowData(false);

  setAttribute(Qt::WA_DeleteOnClose);
  //connect(ui->pushButton,SIGNAL(clicked()),SLOT(slotExit()));
//setStyleSheet("background-color: rgb(200, 220, 210);");
//  ui->exitButton->setStyleSheet("background-color: rgb(200, 220, 210);color:rgb(200, 0, 210);");

  sigLabelMas[0] = new SigLabel(ui->label_32);
  sigLabelMas[1] = new SigLabel(ui->label_33);
  sigLabelMas[2] = new SigLabel(ui->label_34);
  sigLabelMas[3] = new SigLabel(ui->label_35);
  sigLabelMas[4] = new SigLabel(ui->label_36);
  sigLabelMas[5] = new SigLabel(ui->label_37);
  sigLabelMas[6] = new SigLabel(ui->label_38);
  sigLabelMas[7] = new SigLabel(ui->label_39);
  sigLabelMas[8] = new SigLabel(ui->label_40);
  sigLabelMas[9] = new SigLabel(ui->label_41);

  sigLabelMas[10] = new SigLabel(ui->label_24);
  sigLabelMas[11] = new SigLabel(ui->label_25);
  sigLabelMas[12] = new SigLabel(ui->label_26);
  sigLabelMas[13] = new SigLabel(ui->label_27);
  sigLabelMas[14] = new SigLabel(ui->label_28);
  sigLabelMas[15] = new SigLabel(ui->label_29);
  sigLabelMas[16] = new SigLabel(ui->label_30);
  sigLabelMas[17] = new SigLabel(ui->label_31);

  sigLabelMas[18] = new SigLabel(ui->label_18);
  sigLabelMas[19] = new SigLabel(ui->label_19);
  sigLabelMas[20] = new SigLabel(ui->label_20);
  sigLabelMas[21] = new SigLabel(ui->label_21);
  sigLabelMas[22] = new SigLabel(ui->label_22);
  sigLabelMas[23] = new SigLabel(ui->label_23);

  sigLabelMas[24] = new SigLabel(ui->label_10);
  sigLabelMas[25] = new SigLabel(ui->label_11);
  sigLabelMas[26] = new SigLabel(ui->label_12);
  sigLabelMas[27] = new SigLabel(ui->label_13);
  sigLabelMas[28] = new SigLabel(ui->label_14);
  sigLabelMas[64] = new SigLabel(ui->label_15);
  sigLabelMas[29] = new SigLabel(ui->label_16);
  sigLabelMas[30] = new SigLabel(ui->label_17);
  sigLabelMas[31] = new SigLabel(ui->label_18);

  sigLabelMas[32] = new SigLabel(ui->label_0);
  sigLabelMas[33] = new SigLabel(ui->label_1);
  sigLabelMas[34] = new SigLabel(ui->label_2);
  sigLabelMas[35] = new SigLabel(ui->label_3);
  sigLabelMas[36] = new SigLabel(ui->label_4);
  sigLabelMas[37] = new SigLabel(ui->label_5);
  sigLabelMas[38] = new SigLabel(ui->label_6);
  sigLabelMas[39] = new SigLabel(ui->label_7);
  sigLabelMas[40] = new SigLabel(ui->label_8);
  sigLabelMas[41] = new SigLabel(ui->label_9);

  sigLabelMas[42] = new SigLabel(ui->label_42);
  sigLabelMas[43] = new SigLabel(ui->label_43);

  sigLabelMas[44] = new SigLabel(ui->label_44);

  sigLabelMas[45] = new SigLabel(ui->label_45);
  sigLabelMas[46] = new SigLabel(ui->label_46);

  busy=0;
  en = 1;
}

//****************************************************************************************
AvtWindow::~AvtWindow()
{
    delete ui;
}

//****************************************************************************************
void AvtWindow::slotExit()
{
qint8 i;
// for(i=0;i<54;i++)
//    if (sigLabelMas[i]->isToggeled()) sigLabelMas[i]->switchOff(); else sigLabelMas[i]->switchOn();

//if (en==1) en=0;else en=1;
}

//****************************************************************************************
void AvtWindow::screenRefresh(RecordBlackBox elem)
{
    quint8 i;
    quint8 j;
    quint8 byt;
    quint16 l;
    quint16 k;

busy = 1;

    for(i=0;i<4;i++)
    {
        l = 1;
        byt  = elem.InCww1[i];
        for(j=0;j<8;j++)
        {
            k=i*8 + j;
            if ((byt & l)!=0)
            {
                if (sigLabelMas[k] !=0) sigLabelMas[k]->switchOn();
            }else
            {
                if (sigLabelMas[k] !=0) sigLabelMas[k]->switchOff();
            }
            l *= 2;
        }
    }

    for(i=0;i<4;i++)
    {
        l = 1;
        byt  = elem.InCww2[i];
        for(j=0;j<8;j++)
        {
            k=32 + i*8 + j;
            if ( k<47 )
            {
                if ((byt & l)!=0)
                {
                    if (sigLabelMas[k] !=0) sigLabelMas[k]->switchOn();
                }else
                {
                   if (sigLabelMas[k] !=0) sigLabelMas[k]->switchOff();
                }
            }
            l *= 2;
        }
    }

 /*       if (sigLabelMas[56]->isToggeled()) sigLabelMas[64]->switchOn();else sigLabelMas[64]->switchOff();
        if (sigLabelMas[57]->isToggeled()) sigLabelMas[65]->switchOn();else sigLabelMas[65]->switchOff();
        if (sigLabelMas[58]->isToggeled()) sigLabelMas[66]->switchOn();else sigLabelMas[66]->switchOff();

        if (!(sigLabelMas[38]->isToggeled() || sigLabelMas[40]->isToggeled() || sigLabelMas[42]->isToggeled() || sigLabelMas[44]->isToggeled() || sigLabelMas[46]->isToggeled() || sigLabelMas[48]->isToggeled() || sigLabelMas[50]->isToggeled() || sigLabelMas[52]->isToggeled() || sigLabelMas[54]->isToggeled() ))
        {
           sigLabelMas[67]->switchOff();
        } else
        { sigLabelMas[67]->switchOn(); }
*/
    busy = 0;
}

//****************************************************************************************
void AvtWindow::closeEvent(QCloseEvent *event)
{
    qobject_cast<MainWidget *>(this->parent())->setRedrawingMainWindowData(true);
    en =0;
    event->accept();
}

