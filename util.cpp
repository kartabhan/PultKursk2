#include "util.h"

Util::Util(QObject *parent) :
    QObject(parent)
{


}
QString Util::IntAsc(byte sm)
{

    if (sm<10)
    {
     return(QString::number(sm));
    }else
    {

        switch (sm) {
        case 10:
            return "a";
            break;
        case 11:
            return "b";
            break;
        case 12:
            return "c";
            break;
        case 13:
            return "d";
            break;
        case 14:
            return "e";
            break;
        case 15:
            return "f";
            break;

        default:
            return QString::number(0);
            break;
        }
    }

}


byte Util::AscInt(QString sm)
{
    bool ok=false;
    byte iret = sm.toInt(&ok,10);

    if (ok)
    {
     return(iret);
    }else
    {
    if (sm.compare("a") || sm.compare("0a"))
     return 10;else
      if (sm.compare("b") || sm.compare("0b"))
          return 11;else
           if (sm.compare("c") || sm.compare("0c"))
               return 12;else
                if (sm.compare("d") || sm.compare("0d"))
                    return 13;else
                     if (sm.compare("e") || sm.compare("0e"))
                         return 14;else
                          if (sm.compare("f") || sm.compare("0f"))
                              return 15;else
                               return 0;



    }

}

QString Util::Trans1In2(byte wr)
{
  QString stret;

    stret = IntAsc(wr / 16);
    wr = wr % 16;
    stret = stret + IntAsc(wr);
    return stret;

}
QString Util::Trans2In4(word wr)
{
  QString stret;
  word ll;

    ll=wr;
    stret = IntAsc(ll / 4096);
    ll = ll % 4096;
    stret = stret + IntAsc(ll / 256);
    ll = ll % 256;
    stret = stret + IntAsc(ll / 16);
    ll = ll % 16;
    stret = stret + IntAsc(ll);


    return stret;

}
word Util::scKS(QString *strRenamed, byte Nsm)
{
    qint8 i;
    qint32 bt=0;

    for(i=0;i<Nsm;i++)
    {
      bt += strRenamed->at(i).cell();
      if (bt>=65535) bt-=65535;

    }
    return bt;
}
QString Util::timeCharToString(char *chMas)
{
    QString *resultString = new QString();

    *resultString  = QString::fromLatin1(chMas,12);

   return(*resultString);
}

/*



    Public Function OkrugAnl(ByRef por As Byte, ByRef Zn As Double) As Double
        If por = 1 Then OkrugAnl = (Zn * 100 \ 10) / 10
        If por = 2 Then OkrugAnl = (Zn * 1000 \ 10) / 100
        If por = 3 Then OkrugAnl = (Zn * 10000 \ 10) / 1000
        If por = 4 Then OkrugAnl = (Zn * 100000 \ 10) / 10000
        If por = 5 Then OkrugAnl = (Zn * 1000000 \ 10) / 100000
    End Function


 Public Function Trans2In1(ByRef wrAsc As String) As Byte
     Trans2In1 = AscInt(Mid(wrAsc, 1, 1)) * 16
     Trans2In1 = Trans2In1 + AscInt(Mid(wrAsc, 2, 1))
 End Function

 Public Function Trans4In2(ByRef wrAsc As String) As Integer
     Trans4In2 = AscInt(Mid(wrAsc, 1, 1)) * 4096
     Trans4In2 = Trans4In2 + (AscInt(Mid(wrAsc, 2, 1)) * 256)
     Trans4In2 = Trans4In2 + (AscInt(Mid(wrAsc, 3, 1)) * 16)
     Trans4In2 = Trans4In2 + AscInt(Mid(wrAsc, 4, 1))
 End Function



 * Public Sub mnuOnOut_Click(ByVal Sender As System.Object, ByVal e As System.EventArgs) Handles mnuOnOut.Click
     Dim St1, St2 As String
     Dim ll As Byte
     sbrStatus.Panels("������:").Text = "������:"
     If opPusk Then
         OnOut = Not OnOut : sOut1 = 0 : sOut2 = 0 : sOut3 = 0
         St1 = Chr(5) & Chr(33) & Chr(127) : If OnOut Then ll = 1 Else ll = 0
         St2 = "out" & ll : ll = Len(St2) : summa = scKS(St2, ll)
         strTerminate = St1 & Trans2In4(summa) & Trans1In2(ll) & St2 & Chr(13)
         TransmitMK()
         If OnOut = False And nKanal = 7 Then
             fraOper.Visible = False : beOut1 = False : beOut2 = False : beOut3 = False
             OldIndArh1 = 0 : OldIndArh2 = 0 : OldIndArh3 = 0
             Grafics = False : ClearMK(0) ' : ClearMK(1) : ClearMK(2)
             If WorkTest Then mnuTest_Click(mnuTest, New System.EventArgs())
         Else
             mnuOutSgn.Enabled = True : tbrToolBar.Buttons("����� ������� ��������").Enabled = True
             mnuTest.Enabled = True : tbrToolBar.Buttons("���� ���7�2").Enabled = True
             If ArhivArm Then
                 If fArh1 <> "" Then Dim ar1 As New StreamReader(fArh1) : ar1.Close() : fArh1 = ""
                 If fArh2 <> "" Then Dim ar2 As New StreamReader(fArh2) : ar2.Close() : fArh2 = ""
                 If fArh3 <> "" Then Dim ar3 As New StreamReader(fArh3) : ar3.Close() : fArh3 = ""
                 ArhivArm = False
             End If
         End If
     Else
         sbrStatus.Panels("������:").Text = "�������� ������� '��������� ���'"
     End If
 End Sub   */
