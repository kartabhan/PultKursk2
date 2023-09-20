#include "clientsocketadapter.h"
#include <QVector>
#include <QTcpSocket>
#include <QDataStream>

ClientSocketAdapter::ClientSocketAdapter(QObject *parent,qint8 num)
  : SocketAdapter(parent) {  
   if (num ==1)  m_ptcpSocket->connectToHost("192.168.50.254",4001);
   if (num ==2)  m_ptcpSocket->connectToHost("192.168.50.254",4002);
   if (num ==3)  m_ptcpSocket->connectToHost("192.168.50.254",4003);
   if (num ==4)  m_ptcpSocket->connectToHost("192.168.50.254",4004);
}
