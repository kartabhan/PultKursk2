#include "siglabel.h"

SigLabel::SigLabel(QWidget *parent) :
    QLabel(parent)
{
 state = 0;
}
void SigLabel::setState(qint8 st)
{
 state = st;
 if (state==1) switchOn(); else switchOff();
}
void SigLabel::switchOn()
{
     qobject_cast<QLabel *>(this->parent())->setStyleSheet("background-color: rgb(0, 255, 0);");
     state =1;
}
void SigLabel::switchOff()
{
     qobject_cast<QLabel *>(this->parent())->setStyleSheet("background-color: rgb(200, 200, 200);");
     state =0;
}
bool SigLabel::isToggeled()
{
     return(state);
}
