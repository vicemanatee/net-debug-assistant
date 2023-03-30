#ifndef SENDONTIME_H
#define SENDONTIME_H

#include <QMainWindow>
#include <QDate>
#include <QTime>


namespace Ui {
class sendOnTime;
}


//send client/server msg on time
//when the time is arrive, emit signal SsendMessageOntime,
//  include msg, and client/server selection, 1 for client,
//  0 for server.

class timerCaculate : public QObject
{
    Q_OBJECT

public:
    timerCaculate(QWidget* parent = 0);
    QTime targetTime;

public slots:
    void caculateTime();

signals:
    void timeIsUp();

private:


};

class sendOnTime : public QMainWindow
{
    Q_OBJECT

public:
    explicit sendOnTime(QWidget *parent = 0);
    ~sendOnTime();
    timerCaculate* timerC;


private:
    Ui::sendOnTime *ui;
    int sendFrom;
    QString message;

signals:
    void SsendMessageOnTime(QString, int);
    void SstartCaculateTime();
    void SshowWhatGoingToSend(QString, QTime);

private slots:
    void on_cancel_clicked();
    void on_RBsendOnTimeFromClient_clicked();
    void on_confirm_clicked();
    void on_RBsendOnTimeFromServer_clicked();
};

#endif // SENDONTIME_H
