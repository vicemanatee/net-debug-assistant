#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
#include <QList>

class tcpServer : public QTcpServer
{
    Q_OBJECT

public:
    tcpServer(QObject* parent = 0);

protected:
    QList<QTcpSocket*> list_tcpClient;

private:
    QString ip;
    unsigned short port;
    QStringList m_strListData;
    QTcpSocket* socket;
    int sendFileMode;
    int receiveFileSize;
    int writeFileSize;
    int fileRealSize;

signals:
    void SupdateClientData(QString, int);
    void SstartListening();
    void SgetClientMessage(QByteArray);
    void SfileTranslateProgress(int);

private slots:
    void slotGetClientMessage();
    void slotDisconnect();
    void receiveFile(QByteArray clientMessage);
};

#endif // TCPSERVER_H
