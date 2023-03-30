#ifndef MODEL_CLIENT_H
#define MODEL_CLIENT_H

//#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <sendfileinthread.h>
#include <QWidget>

namespace Ui {
class clientSendFileInTHread;
}

class model_client : public QWidget
{
    Q_OBJECT

public:

    explicit model_client(QWidget *parent = 0);
    ~model_client();

    unsigned short port;
    QString ip;
    QByteArray msg;
    QByteArray serverMessage;

    void client();//connect
    QTcpSocket* m_c;
    QThread* fileThread;

private:
    sendFileInThread* sendFile;
    Ui::clientSendFileInTHread *ui;

signals:
    void SgetServerMessage(QByteArray);
    void SsendIPPortToSendFile(unsigned short, QString);
    void SsendFile(QString path);

public slots:
    void serverDisconnected();
    void sendClientMessage();
    void initialSendFile();

private slots:
    void getServerMessage();
    void on_BselectFile_clicked();
    void on_BsendFile_clicked();
    void on_BcancelSendFile_clicked();
};

#endif // MODEL_CLIENT_H
