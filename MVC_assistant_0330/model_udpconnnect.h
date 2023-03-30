#ifndef MODEL_UDPCONNNECT_H
#define MODEL_UDPCONNNECT_H
#include <QUdpSocket>
#include <QWidget>


class model_UDPconnnect : public QWidget
{
    Q_OBJECT

private:
    QUdpSocket* udpSocket;

public:
    model_UDPconnnect(QWidget* parent = 0);
    QHostAddress ip;
    quint16 port;

signals:
    void getUdpMsg(QString);

public slots:
    void readyRead();
    void sendMsg(QString str);
    void closeSocket();
};

#endif // MODEL_UDPCONNNECT_H
