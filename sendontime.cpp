#include "sendontime.h"
#include "ui_sendontime.h"

#include <QDebug>
#include <QThread>


sendOnTime::sendOnTime(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::sendOnTime)
{
    sendFrom = 1;
    ui->setupUi(this);
}

sendOnTime::~sendOnTime()
{
    delete ui;
}

void sendOnTime::on_cancel_clicked()
{
    this->hide();
}

void sendOnTime::on_RBsendOnTimeFromClient_clicked()
{
    if(ui->RBsendOnTimeFromClient->isChecked())
        sendFrom = 1;
    else
        sendFrom = 0;
}

void sendOnTime::on_confirm_clicked()
{
    this->hide();

    message = ui->LEsend->toPlainText();
    qDebug()<<"send sth when time is up"<<message;
    timerC = new timerCaculate;
    QThread *thread = new QThread;
    thread->start();
    timerC->moveToThread(thread);
    timerC->targetTime = ui->TEsetTimeToSend->time();
    connect(this, &sendOnTime::SstartCaculateTime,
            timerC, &timerCaculate::caculateTime);
    connect(timerC, &timerCaculate::timeIsUp,
            this, [=](){
        emit SsendMessageOnTime(message, sendFrom);
    });

    emit SshowWhatGoingToSend(message, timerC->targetTime);
    emit SstartCaculateTime();
}

timerCaculate::timerCaculate(QWidget *parent):
    QObject(parent)
{

}

//when time is up emit signal
void timerCaculate::caculateTime()
{
    qDebug()<<"sendMsg on time Thread:"<<QThread::currentThread();
    while (1)
    {
        QTime currentTime = QTime::currentTime();
            if(currentTime == targetTime)
            {
                emit timeIsUp();
                break;
            }
    }
}

void sendOnTime::on_RBsendOnTimeFromServer_clicked()
{
    if(ui->RBsendOnTimeFromClient->isChecked())
        sendFrom = 1;
    else
        sendFrom = 0;
}
