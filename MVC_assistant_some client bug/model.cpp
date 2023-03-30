#include "model.h"
#include <QThread>
#include <QFile>

model::model(QObject *parent) :
    QTcpServer(parent)
{
    m_s = new QTcpServer(this);
    m_tcp = new QTcpSocket(this);
    port = 8899;
    sendFileMode = 0;
}

void model::activateListen()
{
    m_s->listen(QHostAddress::Any, port);

    emit SstartListening();

    connect(m_s, &QTcpServer::newConnection,
            this, &model::whenListen);
}

void model::whenListen()
{
    m_tcp = m_s->nextPendingConnection();
    //m_tcp->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 1024*1024);
    //qDebug()<<"server socket receive buffer size"<<m_tcp->socketOption(QAbstractSocket::ReceiveBufferSizeSocketOption);

    connect(m_tcp, &QTcpSocket::readyRead,
            this, &model::getClientMessage);
    connect(m_tcp, &QTcpSocket::disconnected,
            this, &model::clientDisconnected);
}

void model::getClientMessage()
{
    clientMessage = m_tcp->readAll();

    //qDebug()<<"get clientMessage Size"<<clientMessage.size();

    if(QString(clientMessage).startsWith("FILE&&&")){
        sendFileMode = 1;
        receiveFileSize = 0;
        writeFileSize = 0;
    }
    if(!clientMessage.isEmpty()){
        if(sendFileMode == 0)
            emit SgetClientMessage(clientMessage);
        else
            receiveFile();
    }
}

void model::sendServerMessage(int index)
{
    m_tcp->write(msg);
}

void model::clientDisconnected()
{
    m_tcp->disconnectFromHost();
    qDebug()<<"client disconnect";
    m_tcp->close();
}

void model::receiveFile()
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

void model::serverError()
{

}

tcpServer::incomingConnection(qintptr socketDescriptor())
{

}
