#include "tcpserver.h"

#include <QTcpSocket>
#include <QFile>

tcpServer::tcpServer(QObject *parent) :
    QTcpServer(parent)
{
    port = 8899;
    sendFileMode = 0;
    this->listen(QHostAddress::Any, port);
    emit SstartListening();
}



void tcpServer::slotGetClientMessage()
{
    QByteArray clientMessage = socket->readAll();

    qDebug()<<"get client Message Size"<<clientMessage.size();

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

void tcpServer::slotDisconnect()
{
    for(int index = 0;
        index < list_tcpClient.size();
        index ++)
    {
        QTcpSocket* temp = list_tcpClient.at(index);
        if (temp->state() != QAbstractSocket::ConnectedState)
            list_tcpClient.removeAt(index);
        qDebug()<<"list size"<<list_tcpClient.size();
    }
}

void tcpServer::receiveFile(QByteArray clientMessage)
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


