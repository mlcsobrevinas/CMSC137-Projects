#include "server.h"

Server::Server()
{

    connect(this, &QUdpSocket::readyRead,
            this, &Server::readPendingDatagrams);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout,
            this, &Server::resendData);
    this->currentSeqNum = 10;
    this->timeout = 4000;
    this->state=0;
    this->header = new Header();
    srand(time(NULL)+5);
    this->timeout += rand()%8;

    this->drop = 0;
}

bool Server::listen(quint16 port)
{
    qDebug("Server[%d]: Listening on port %d.", this->state, port);
    return this->bind(QHostAddress::LocalHost, port);
}


void Server::sendData(const QByteArray &data)
{
    if(data.size()>0){
        for(int i=0;i<data.size();i+=4)
            toSend.push_back(data.mid(i,4));
    }

    if(state == 2){
    
        if(currentSeqNum-baseSeqNum < toSend.size() && !timer->isActive()){
            QByteArray frame = toSend[currentSeqNum-baseSeqNum];
            this->writeDatagram(header->getByteArray() + frame,peer,peerPort);
            this->timer->start(this->timeout);
        } else if(needToAck){
            this->writeDatagram(header->getByteArray(),peer,peerPort);
        }
    }
}

void Server::readPendingDatagrams()
{
    bool seqNumIncreased = false;
    while (this->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(this->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        this->readDatagram(datagram.data(), datagram.size(),
                           &sender, &senderPort);


        header->readFrom(datagram);

        if(state==0 && header->syn() && !header->ack()){
            qDebug("Server[%d]: SYN Recieved.", state);
            header->swapNums();
            header->incrementAckNum();
            header->setSeqNum(currentSeqNum);
            currentSeqNum = header->seqNum();
            currentAckNum = header->ackNum();

            header->setAck(true);
            this->writeDatagram(header->getByteArray(), sender, senderPort);
            state++;
        } else if(state==1 && !header->syn() && header->ack()){
            header->swapNums();
            if(header->seqNum() != currentSeqNum+1 || header->ackNum() != currentAckNum){
                qDebug("Server[%d]: Improper ACK Recieved.", state);
                state = 0;
            } else {
                qDebug("Server[%d]: Proper ACK Recieved.", state);

                peer=sender;
                peerPort=senderPort;

                state++;
                currentSeqNum = header->seqNum();
                baseSeqNum = header->seqNum();

                if(!this->hasPendingDatagrams())
                    sendData();
                qDebug("Server: Fully Connected");
            }
        } else if(state==2 && !header->syn() && header->ack()){
            QByteArray data =  datagram.mid(12);
            bool drop = false;
            if(rand()%4 >= this->drop){
            }else{

                if(data.size()>0) qDebug() << "\e[1;31mS: ###### DROPPED" << data << "\e[m";
                drop=true;
            }
            header->swapNums();


            if(data.size() > 0){
                if(!drop) qDebug() << "\e[1;31mS: ###### Received " << data << "\033[0m";
                needToAck = true;
                if(!drop){
                    header->incrementAckNum();
                    currentSeqNum=header->seqNum();
                }
            }

            if(header->seqNum() == currentSeqNum+1){
                
                currentSeqNum = header->seqNum();
                this->timer->stop();
            }

            sendData();
            needToAck = false;
        } else if(state==2 && header->fin()){
            qDebug("FIN Received. Sending ACK and then FIN");
            header->setAck(true);
            header->setSyn(false);
            this->writeDatagram(header->getByteArray(), sender, senderPort);
            header->setAck(false);
            header->setFin(true);
            this->writeDatagram(header->getByteArray(), sender, senderPort);
            state=5;
        } else if(state==3 && header->ack()){
            qDebug("ACK Received. Waiting for FIN");
            state = 4;
        } else if(state==4 && header->fin()){
            qDebug("Fin Received. Sending ACK");
            header->setAck(true);
            header->setSyn(false);
            header->setFin(false);
            this->writeDatagram(header->getByteArray(), sender, senderPort);
            state = 9;
        } else if(state==5 && header->ack()){
            qDebug("ACK Received.");
            state = 9;
        }

        if(state == 9){
            qDebug("Connection terminated");
        }
    }
}

void Server::resendData()
{
    qDebug("\e[1;31mServer: Sending previous data failed. Resending..\e[0m");
    this->timer->stop();
    this->sendData();
}


void Server::disconnect(){
    qDebug() << "Now disconnecting...";
    qDebug() << "Sending FIN";
    header->setAck(false);
    header->setSyn(false);
    header->setFin(true);
    this->writeDatagram(header->getByteArray(), peer, peerPort);
    state=3;
}

void Server::setDrop(quint16 drop){
    this->drop = drop;
}
