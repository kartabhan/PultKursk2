#ifndef CLIENTSOCKETADAPTER_H
# define CLIENTSOCKETADAPTER_H

# include "net/socketadapter.h"
#include "debug.h"
class ClientSocketAdapter : public SocketAdapter {
  Q_OBJECT
public:
  explicit ClientSocketAdapter(QObject *parent,qint8 num);
};

#endif // CLIENTSOCKETADAPTER_H
