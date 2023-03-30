#ifndef MODEL_H
#define MODEL_H

#include <QTcpServer>
#include <QObject>
#include <QTcpSocket>
#include <QList>
#include <QMap>


class tcpServer : public QTcpServer
{
protected:
    incomingConnection(qintptr socketDescriptor());

};

class model : public QTcpServer
{
    Q_OBJECT

public:

    model(QObject *parent = 0);

    unsigned short port;
    QByteArray clientMessage;
    QByteArray serverMessage;
    QByteArray msg;
    QString ip;

    void activateListen();
    void sendServerMessage();
    QTcpServer* m_s;
    QTcpSocket* m_tcp;

private:
    int sendFileMode;
    int receiveFileSize;
    int writeFileSize;
    int fileRealSize;
    void serverError();

signals:
    void SgetClientMessage(QByteArray);
    void SgetClientFile();
    void SfinishClientFile();
    void SfileTranslateIncomplete();
    void SfileTranslateProgress(int);
    void SclientDisconnect();
    void SstartListening();

public slots:
    void clientDisconnected();

private slots:
    void whenListen();
    void getClientMessage();
    void receiveFile();

};

#endif // MODEL_H
