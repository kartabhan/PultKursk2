#include "socketadapter.h"

#include <QVector>
#include <QTcpSocket>
#include <QDataStream>

SocketAdapter::SocketAdapter(QObject *parent, QTcpSocket *pSock)
  : ISocketAdapter(parent), m_msgSize(-1) {
  if (0 == pSock)
    m_ptcpSocket = new QTcpSocket(this);
  else
    m_ptcpSocket = pSock;
  connect(m_ptcpSocket, SIGNAL(readyRead()), this, SLOT(on_readyRead()));
  connect(m_ptcpSocket, SIGNAL(disconnected()), this, SLOT(on_disconnected()));
}

SocketAdapter::~SocketAdapter() {
}

void SocketAdapter::on_readyRead() {
  QString *buff=new QString();  
  QDataStream *stream= new QDataStream(m_ptcpSocket);
  char *pp=new char[5000];


  while(true)
  {
      uint le=m_ptcpSocket->bytesAvailable();
      if ( le>0)
      {

       stream->readRawData(pp,le);
       buff = new QString(pp);
       buff->truncate(le);
       emit message(buff);

      }else
      {return;}

  }


}

void SocketAdapter::sendString(const QString * str) {
  /*QByteArray block;
  QDataStream sendStream(&block, QIODevice::ReadWrite);

  sendStream << quint16(0) << str;

  sendStream.device()->seek(0);
  sendStream << (quint16)(block.size() - sizeof(quint16));
*/
  char *ptr =str->toLatin1().data();
  m_ptcpSocket->write(ptr);
}

void SocketAdapter::on_disconnected() {
  m_ptcpSocket->deleteLater();
  emit disconnected();
}
