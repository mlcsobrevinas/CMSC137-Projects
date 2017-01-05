#include "header.h"
#include <QDebug>

Header::Header()
{
    header.seqNum = qToBigEndian(1);
    header.ackNum = qToBigEndian(2);

    header.offset = 3;
    header.flags = 0;

    header.window = qToBigEndian((quint16)5);
}

Header::Header(QByteArray &data)
{
    readFrom(data);
}

void Header::readFrom(QByteArray &data)
{
    QByteArray headerData = data.left(12);
    if(data.size()>=4*3){
        this->header = *((HeaderData*)(headerData.data()));
    }
}

bool Header::syn(){
    return ((header.flags >> 1) & 1) == 1;
}

bool Header::fin(){
    return ((header.flags) & 1) == 1;
}

bool Header::ack(){
    return ((header.flags >> 4) & 1) == 1;
}

QByteArray Header::getByteArray(){
    return QByteArray::fromRawData((const char*)&header, sizeof(struct HeaderData));
}

quint32 Header::seqNum()
{
    return qFromBigEndian(header.seqNum);
}

quint32 Header::ackNum()
{
    return qFromBigEndian(header.ackNum);
}

quint8 Header::offset()
{
    return header.offset;
}

quint16 Header::window()
{
    return qFromBigEndian(header.window);
}

void Header::setSeqNum(quint32 value)
{
    this->header.seqNum = qToBigEndian(value);
}

void Header::setAckNum(quint32 value)
{
    this->header.ackNum = qToBigEndian(value);
}

void Header::setOffset(quint8 value)
{
    this->header.offset = qToBigEndian(value);
}

void Header::setWindow(quint16 value)
{
    this->header.window = qToBigEndian(value);
}

void Header::incrementSeqNum(quint32 value)
{
    this->header.seqNum = qToBigEndian(qFromBigEndian(this->header.seqNum) + value);
}

void Header::incrementAckNum(quint32 value)
{
    this->header.ackNum = qToBigEndian(qFromBigEndian(this->header.ackNum) + value);
}

void Header::swapNums()
{
    quint32 tmp = this->header.seqNum;
    this->header.seqNum = this->header.ackNum;
    this->header.ackNum = tmp;
}

void Header::setSyn(bool value)
{
    if(value)
        this->header.flags |= (1<<1);
    else
        this->header.flags &= (~(1<<1));
}

void Header::setAck(bool value)
{
    if(value)
        this->header.flags |= (1<<4);
    else
        this->header.flags &= (~(1<<4));
}

void Header::setFin(bool value)
{
    if(value)
        this->header.flags |= (1);
    else
        this->header.flags &= (~(1));
}
