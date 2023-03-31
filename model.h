#ifndef MODEL_H
#define MODEL_H

#include <QTcpServer>
#include <QObject>
#include <QTcpSocket>
#include <QList>
#include <QMap>


class tcpServer : public QTcpServer
{
    Q_OBJECT

protected:
    void incomingConnection(qintptr socketDescriptor);

signals:
    void SnewConnect(qintptr socketDescriptor);
    void SnewMsg(QByteArray msg);
    void SclientDisconnect();

};

class model : public QTcpServer
{
    Q_OBJECT

public:

    model(QObject *parent = 0);

    unsigned short port;
    QByteArray serverMessage;
    QByteArray msg;
    QString ip;


    void sendServerMessage(int index);
    tcpServer* m_s;

private:
    int sendFileMode;
    int receiveFileSize;
    int writeFileSize;
    int fileRealSize;
    void serverError();
    QList<QTcpSocket*> socketList;

signals:
    void SgetClientMessage(QByteArray);
    void SgetClientFile();
    void SfinishClientFile();
    void SfileTranslateIncomplete();
    void SfileTranslateProgress(int);
    void SclientDisconnect();
    void SstartListening();
    void SnewMessage(QByteArray);
    void SsocketListUpdate(QList<QTcpSocket*>);
    void SserverClose();

public slots:
    void activateListen();
    void clientDisconnected(qintptr socketDescriptor);
    void serverClose();
    void slotSendServerMessage(QByteArray message, int index);

private slots:
    void getClientMessage(QByteArray clientMessage);
    void receiveFile(QByteArray clientMessage);
    void newConnect(qintptr socketDescriptor);

};

#endif // MODEL_H
