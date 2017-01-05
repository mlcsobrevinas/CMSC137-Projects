#include "client.h"

Client::Client()
{

    connect(this, &Client::readyRead,
            this, &Client::readPendingDatagrams);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout,
            this, &Client::resendData);
    this->currentSeqNum = 90;
    this->timeout = 4000+20;
    this->state=0;
    header = new Header();
    srand(time(NULL));
    this->timeout += rand()%8;
    this->needToAck=false;
}


void Client::connectTo(const QHostAddress &address, const quint16 &port)
{
    header->setSeqNum(currentSeqNum);
    header->setSyn(true);
    header->setAck(false);

    this->state=0;
    qDebug() << "Client: Connecting to " << address << ":" << port;
    qDebug() << "Client: Pseudo-Connected";
    this->writeDatagram(header->getByteArray(), address,port);
    state++;

}

void Client::sendData(const QByteArray &data)
{
    if(data.size()>0){
        qDebug() << "Adding data..";
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
        } else if(!timer->isActive()){
            std::string msg;
            std::cout << "Enter message:";
            std::getline(std::cin, msg);

            if(msg.length()==0) this->disconnect();

            sendData(QString::fromStdString(msg).toUtf8());
        }
    }
}

void Client::readPendingDatagrams()
{
    bool seqNumIncreased = false;
    while (this->hasPendingDatagrams()) {
        needToAck = false;
        QByteArray datagram;
        datagram.resize(this->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        this->readDatagram(datagram.data(), datagram.size(),
                           &sender, &senderPort);

        header->readFrom(datagram);

        if(state==1 && header->syn() && header->ack()){
            qDebug() << "Client: SYN+ACK Received";

            header->setSyn(false);
            header->incrementSeqNum();

            header->swapNums();

            currentSeqNum = header->seqNum();
            baseSeqNum = header->seqNum();
            this->writeDatagram(header->getByteArray(), sender, senderPort);

            peer=sender;
            peerPort=senderPort;

            state++;
            qDebug() << "Client: Fully connected.";

            sendData();
        } else if(state==2 && !header->syn() && header->ack()){
            QByteArray data =  datagram.mid(12);
            bool drop = false;
            if(rand()%4 != 0){
            }else{

                if(data.size()>0) qDebug() << "\e[1;36mC: ###### DROPPED" << data << "\e[m";
                drop=true;
            }
            header->swapNums();


            if(data.size() > 0){
                needToAck = true;
                if(!drop){
                    qDebug() << "\e[1;34mC: ###### Received " << data << "\033[0m";
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

void Client::resendData()
{
    qDebug("\e[1;34mClient: Sending previous data failed. Resending..\e[0m");
    this->timer->stop();
    this->sendData();
    this->flush();
}

void Client::disconnect(){
    qDebug() << "Now disconnecting...";
    qDebug() << "Sending FIN";
    header->setAck(false);
    header->setSyn(false);
    header->setFin(true);
    this->writeDatagram(header->getByteArray(), peer, peerPort);
    state=3;
}
