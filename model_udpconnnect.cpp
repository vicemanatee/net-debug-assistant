#include "model_udpconnnect.h"


model_UDPconnnect::model_UDPconnnect(QWidget *parent):
    QWidget(parent)
{
    udpSocket = new QUdpSocket;
    ip = "192.168.200.1";
    port = 8899;
    connect(udpSocket, &QUdpSocket::readyRead,
            this, &model_UDPconnnect::readyRead);
}

void model_UDPconnnect::readyRead()
{
    QByteArray data;
    while(udpSocket->hasPendingDatagrams())
    {
        data.resize(udpSocket->bytesAvailable());
        udpSocket->readDatagram(data.data(), data.size(),
                                &ip, &port);
        emit getUdpMsg(QString(data));
    }
}

void model_UDPconnnect::sendMsg(QString str)
{
    int succeed = udpSocket->writeDatagram(str.toUtf8(), ip,//QHostAddress::Broadcast,
                             port);
    qDebug()<<"success?"<<succeed<<" ip:"<<ip;
}

void model_UDPconnnect::closeSocket()
{
    udpSocket->abort();
}


