#include "verticallabel.h"
#include <QPainter>
#include <QMouseEvent>

VerticalLabel::VerticalLabel(QWidget *parent) :
    QLabel(parent)
{
}

VerticalLabel::VerticalLabel(const QString &text,quint8 num, QWidget *parent) :
    QLabel(text,parent)
{
    setStyleSheet("background-color: rgb(200, 200, 200);");
    setLineWidth(1);
    setFrameShape(QFrame::Box);
    setAlignment(Qt::AlignCenter);
    par = parent;
    myNumber = num;





}

void VerticalLabel::paintEvent(QPaintEvent *)
{
        QPainter painter(this);
        painter.setPen(Qt::black);
        painter.setBrush(Qt::Dense1Pattern);

        painter.rotate(270);
        painter.drawText(-75,20,text());
}

void VerticalLabel::setGeometry(int x, int y, int w, int h)
{
    blueStrip = new QLabel(par);
    blueStrip->setVisible(false);
    blueStrip->setGeometry( x,y,40,5);
    blueStrip->setStyleSheet("background-color: rgb(50, 50, 255);");

    QLabel::setGeometry(x,y,w,h);
}

QSize VerticalLabel::minimumSizeHint() const
{
    QSize s = QLabel::minimumSizeHint();
    return QSize(s.height(),s.width());
}

QSize VerticalLabel::sizeHint() const
{
    QSize s = QLabel::sizeHint();
    return QSize(s.height(),s.width());
}
QString VerticalLabel::getErrorText()
{

    return("Ошибка " + this->text());
}
quint8 VerticalLabel::getLabelNumber()
{
    return(this->myNumber);
}

void VerticalLabel::setState(quint8 state)
{

    switch (state) {
    case 0:
        this->setStyleSheet("background-color: rgb( 195, 195, 195);");
         blueStrip->setVisible(false);
        break;

    case 1:
        this->setStyleSheet("background-color: rgb( 195, 195, 195);");
        blueStrip->setVisible(true);
        break;

    case 2:

        this->setStyleSheet("background-color: rgb( 255, 255, 0);");
         blueStrip->setVisible(false);
        break;

    case 3:

        this->setStyleSheet("background-color: rgb( 255, 50, 50);");
         blueStrip->setVisible(false);
        break;


    default:
        break;
    }


}

void VerticalLabel::mousePressEvent ( QMouseEvent * event )
{
    if (event->type()== QEvent::MouseButtonPress)
    {
        emit clicked();
    }
}
