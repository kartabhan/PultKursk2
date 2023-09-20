#ifndef SIGLABEL_H
#define SIGLABEL_H

#include <QLabel>

class SigLabel : public QLabel
{
    Q_OBJECT
public:
    explicit SigLabel(QWidget *parent = 0);

    void setState(qint8 st);
    void switchOn();
    void switchOff();
    bool isToggeled();

signals:

public slots:

private:
    qint8 state;

};

#endif // SIGLABEL_H
