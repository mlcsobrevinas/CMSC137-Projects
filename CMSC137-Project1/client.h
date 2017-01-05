#ifndef Client_H
#define Client_H

#include <QMutex>
#include <QQueue>
#include <QTimer>
#include <QUdpSocket>
#include <header.h>
#include <cstdlib>
#include <ctime>
#include <iostream>



class Client : public QUdpSocket
{
public:
    Client();
    void connectTo(const QHostAddress &address, const quint16 &port);
    void sendData(const QByteArray &data = QByteArray());
    void disconnect();
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

    quint16 lastReceived;

    QHostAddress peer;
    quint16 peerPort;
};

#endif 
