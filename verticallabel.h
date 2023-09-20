#ifndef VERTICALLABEL_H
#define VERTICALLABEL_H

#include <QLabel>
#include <QString>

class VerticalLabel : public QLabel
{
    Q_OBJECT
public:
    explicit VerticalLabel(QWidget *parent = 0);
    explicit VerticalLabel(const QString &text,quint8 num = 0, QWidget *parent = 0);
    QString getErrorText();
    quint8 getLabelNumber();
    void setGeometry(int x, int y, int w, int h);

    void setState(quint8 state);

protected:

    void paintEvent(QPaintEvent *);\
    QSize sizeHint() const;
    QSize minimumSizeHint() const;


private:
    quint8 myNumber;

    QWidget *par;

    QLabel *blueStrip;




signals:
void clicked();



protected:
void mousePressEvent ( QMouseEvent * event ) ;



};

#endif // VERTICALLABEL_H
