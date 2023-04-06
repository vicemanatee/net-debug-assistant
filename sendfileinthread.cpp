#include "sendfileinthread.h"

#include <QFile>
#include <QFileInfo>
#include <QThread>
#include <QDebug>

sendFileInThread::sendFileInThread()
{

}

void sendFileInThread::sendFile(QString path)
{
    qDebug()<<"opening file in "<<QThread::currentThread();
    QFile file(path);
    QFileInfo info(path);
    int fileSize = info.size();
    int sendSize = 0;
    int sendFileProgress;
    file.open(QFile::ReadOnly);

    double len = 0.5;



    do{
        /*
        if (len == 0.5)
            emit SreadTextLine(startSendFile.toUtf8());*/
        char buffer[2*1024] = {0};
        len = 0;
        len = file.read(buffer,sizeof(buffer)-2);
        emit SreadTextLine(buffer);

        sendSize += len;

        sendFileProgress = 100*sendSize/fileSize;
        emit ScurPercent(sendFileProgress);
        QThread::sleep(1);
    }while(len > 0);
    //emit SreadTextLine(finishSendFile.toUtf8());

    qDebug()<<"send total size"<<sendSize;
    qDebug()<<"file real size"<<fileSize;

    if(sendSize = fileSize)
        emit SfileSendSuccess();
}


void sendFileInThread::getIPPort(unsigned short serverPort, QString serverIP)
{
    port = serverPort;
    ip = serverIP;
}
