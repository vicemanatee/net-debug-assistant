#ifndef SENDFILEINTHREAD_H
#define SENDFILEINTHREAD_H

#include <QWidget>

class sendFileInThread :public QObject
{
    Q_OBJECT

public:
    sendFileInThread();

private:
    unsigned short port;
    QString ip;

signals:
    void SreadTextLine(QByteArray line);
    void ScurPercent(int num);
    void SfileSendSuccess();

public slots:
    void sendFile(QString path);
    void getIPPort(unsigned short , QString);
};

#endif // SENDFILEINTHREAD_H
