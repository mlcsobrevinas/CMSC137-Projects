#ifndef Server_H
#define Server_H

#include <QMutex>
#include <QQueue>
#include <QTimer>
#include <QUdpSocket>
#include <header.h>
#include <cstdlib>
#include <ctime>



class Server : public QUdpSocket
{
public:
    Server();
    bool listen(quint16 port);
    void sendData(const QByteArray &data = QByteArray());
    void disconnect();
    void setDrop(quint16 drop);
public slots:
    void readPendingDatagrams();
    void resendData();
private:
    int state;
    Header *header;
    quint32 currentSeqNum;
    quint32 currentAckNum;
    quint32 timeout;

    QMutex isSending;

    quint32 baseSeqNum;
    QList<QByteArray> window;
    QQueue<QByteArray> toSend;
    QTimer *timer;
    bool needToAck;

    QHostAddress peer;
    quint16 peerPort;

    quint16 drop;
};

#endif 
