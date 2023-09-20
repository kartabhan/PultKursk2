#ifndef MAINWIDGET_H
# define MAINWIDGET_H

#include <QPushButton>
#include <QWidget>
#include <QLabel>
#include <QMenuBar>
#include <QToolBar>
#include <QTimer>
#include <QTime>
#include <QDate>
#include <QFileDialog>
#include <QTextCodec>
#include <QScopedPointer>
#include <QMessageBox>
#include <QProcess>
#include "recordblackbox.h"
#include "graph/avtwindow.h"
#include "debug.h"

namespace Ui {
  class Form;
}

class ISocketAdapter;
class ShowData;
class RecordStorage;
class FailStorage;
class MainWidget : public QWidget {
  Q_OBJECT
public:
  explicit MainWidget(QWidget *parent = 0);

public:
    void setRedrawingMainWindowData(bool flag);
private :
    void showData1();
    void showData2();
    void showData3();
    void showData4();
    void showGraphWindow(qint8 chan);
    bool isRedrawingMainWindowData;
    void archiveBase();
    void cleanArchives();
    void EXIT();
    void sendRS232Time();

    void clearGraphWindow(qint8 chan);

    qint8 timeRecArchive1;
    qint8 timeRecArchive2;
    qint8 timeRecArchive3;
    qint8 timeRecArchive4;
    quint8 needSendTime;
    bool archIsRared;
    bool archIsSaved;
    QString userName;
    QString homeDir;

    const quint64 MAX_ARCHIVES_IN_MB=5500;

    bool wasDisconnectMK1;
    bool wasDisconnectMK2;
    bool wasDisconnectMK3;
    bool wasDisconnectMK4;

    bool isNewMsg1=false;
    bool isNewMsg2=false;
    bool isNewMsg3=false;
    bool isNewMsg4=false;

public slots:

  void on_new_element_mk1(RecordBlackBox element);
  void on_new_element_mk2(RecordBlackBox element);
  void on_new_element_mk3(RecordBlackBox element);  
  void on_new_element_mk4(RecordBlackBox element);
  void updateTimer500mls();

protected:

  QString ADMINPASS;
  QString enterPassword;
  qint8 passAction;
  Ui::Form *m_pForm;
  QToolBar *mainToolBar;
  QMenuBar *mainFileMenu; 

  AvtWindow *avtWidget;
  ISocketAdapter *m_pSock1;
  ISocketAdapter *m_pSock2;
  ISocketAdapter *m_pSock3;
  ISocketAdapter *m_pSock4;
  ShowData *m_pShowdata;
  QLabel *DsMas[288];
  QLabel *DsOutMas[32*3];
  QLabel *dsAn[72];
  QLabel *AnIn[72];
  RecordStorage *m_pRecordStorage;
  FailStorage *failStorage;
  QTimer *timer500mls;
  QTextCodec *codec;
  QAction *saveAction;

private slots:
  void slotShowAvtWidget();
  void slotSaveToFlash();
  void slotShowGraphicCh1();
  void slotShowGraphicCh2();
  void slotShowGraphicCh3();
  void slotShowArchive();
  void slotShowSpaceMess();
  void slotSendOnOut();
  void slotExit();

  void slotPassButt0();
  void slotPassButt1();
  void slotPassButt2();
  void slotPassButt3();
  void slotPassButt4();
  void slotPassButt5();
  void slotPassButt6();
  void slotPassButt7();
  void slotPassButt8();
  void slotPassButt9();
  void slotPassButt10();
  void slotPassButt11();

 /*void on_pushButton_clicked();
  void on_InSigButt_clicked();
  void on_InSigButt_triggered(QAction *arg1);*/

};

#endif // MAINWIDGET_H
