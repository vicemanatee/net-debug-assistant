#include "model_udpconnnect.h"


model_UDPconnnect::model_UDPconnnect(QWidget *parent):
    QWidget(parent)
{
    udpSocket = new QUdpSocket;
    ip = "192.168.1.25";
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
    udpSocket->writeDatagram(str.toUtf8(), QHostAddress::Broadcast,
                             port);
    qDebug()<<"send Message"<< str <<"ip"<<ip;
}

void model_UDPconnnect::closeSocket()
{
    udpSocket->abort();
}


