#include "model.h"
#include <QThread>
#include <QFile>

model::model(QObject *parent) :
    QTcpServer(parent)
{
    m_s = new tcpServer;
    port = 8899;
    sendFileMode = 0;
}

void model::activateListen()
{
    m_s->listen(QHostAddress::Any, port);

    emit SstartListening();

    connect(m_s, &tcpServer::SnewConnect,
            this,&model::newConnect);

}

void model::getClientMessage(QByteArray clientMessage)
{
    if(QString(clientMessage).startsWith("FILE&&&")){
        sendFileMode = 1;
        receiveFileSize = 0;
        writeFileSize = 0;
    }
    if(!clientMessage.isEmpty()){
        if(sendFileMode == 0)
            emit SgetClientMessage(clientMessage);
        else
            receiveFile(clientMessage);
    }
}

void model::sendServerMessage(int index)
{
    if(index == 0){
        //sending to all client
        for(int j = 0;
            j < socketList.size();
            j++){
            QTcpSocket* m_tcp = socketList.at(j);
            m_tcp->write(msg);
        }
    }
    else
    {
        QTcpSocket* m_tcp = socketList.at(index - 1);
        m_tcp->write(msg);
    }
}

void model::clientDisconnected(qintptr socketDescriptor)
{
    for(int j = 0;
        j < socketList.size();
        j++){
        if (socketList.at(j)->socketDescriptor() == socketDescriptor)
            socketList.removeAt(j);
    }
}

void model::receiveFile(QByteArray clientMessage)
{
    receiveFileSize += clientMessage.size();
    QFile* file = new QFile("receive file from client.txt");
    file->open(QFile::Append);
    QString temp = QString(clientMessage);
    //qDebug()<<"receiving file in thread:"<<QThread::currentThread();
    //qDebug()<<"get file content"<<temp;
    QString content = temp;
    if(temp.startsWith("FILE&&&")){
        content = content.section('|',2,-1);
        fileRealSize = temp.section('|',1,1).toInt();
    }
    if(temp.endsWith("FILEFINISH&&&")){
        content = content.section('|',0,-2);
        sendFileMode = 0;
    }
    qDebug()<<"write in file size"<<content.toUtf8().size();
    file->write(content.toUtf8());
    writeFileSize += content.toUtf8().size();
    file->close();

    int sendFileProgress = writeFileSize / fileRealSize;
    emit SfileTranslateProgress(sendFileProgress);

    if(sendFileMode == 0){
        qDebug()<<"file real size"<<fileRealSize;
        qDebug()<<"total file receive size"<<receiveFileSize;
        qDebug()<<"total file write size"<<writeFileSize;
    }
}

void model::newConnect(qintptr socketDescriptor)
{
    QTcpSocket* m_tcp = new QTcpSocket;
    m_tcp->setSocketDescriptor(socketDescriptor);
    socketList.append(m_tcp);
    qDebug()<<"socket state:"<<m_tcp->state();
    emit SsocketListUpdate(socketList);
    connect(m_tcp, &QTcpSocket::readyRead,
            this, [=](){
        QByteArray msg = m_tcp->readAll();
        //qDebug()<<"get msg"<<QString(msg);
        getClientMessage(msg);
    });
    connect(m_tcp, &QTcpSocket::disconnected,
            this, [=](){
        qDebug()<<"disconnect";
        clientDisconnected(m_tcp->socketDescriptor());
        emit SsocketListUpdate(socketList);
    });
    qDebug()<<"list size:"<<socketList.size();
}

void model::serverError()
{

}

void tcpServer::incomingConnection(qintptr socketDescriptor)
{
    emit SnewConnect(socketDescriptor);
    qDebug()<<"new socket"<<socketDescriptor;
}
