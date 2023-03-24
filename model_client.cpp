#include "model_client.h"
#include "ui_clientSendFileInThread.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QThread>


model_client::model_client(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::clientSendFileInTHread)
{
    port = 8899;
    ip = "127.0.0.1";
    ui->setupUi(this);
    m_c = new QTcpSocket(this);
}

model_client::~model_client()
{
    delete ui;
}

void model_client::client()//connect
{

    //qDebug()<<"client Socket"<<m_c;
    m_c->connectToHost(QHostAddress(ip), port);

    connect(m_c, &QTcpSocket::readyRead,
            this, &model_client::getServerMessage);

    connect(m_c, &QTcpSocket::disconnected,
            this, &model_client::serverDisconnected);
}

void model_client::sendClientMessage()
{
    m_c->write(msg);
    //qDebug()<<"client send Thread"<<QThread::currentThread();
}

void model_client::getServerMessage()
{
    serverMessage = m_c->readAll();
    //qDebug()<<"client thread"<<QThread::currentThread();
    emit SgetServerMessage(serverMessage);
}

void model_client::serverDisconnected()
{
    m_c->disconnectFromHost();
    m_c->close();
    //m_c->deleteLater();
}

void model_client::initialSendFile()
{
    sendFile = new sendFileInThread;
    QThread* t = new QThread;
    ui->progressBar->setValue(0);
    sendFile->moveToThread(t);
    t->start();

    //send path to translate class
    //start to read & translate
    connect(this, &model_client::SsendFile,
            sendFile, &sendFileInThread::sendFile);

    //m_c send segment
    connect(sendFile, &sendFileInThread::SreadTextLine,
            this, [=](QByteArray line){
        m_c->write(line);
    });

    //update progress
    connect(sendFile, &sendFileInThread::ScurPercent,
            ui->progressBar, &QProgressBar::setValue);

    connect(sendFile, &sendFileInThread::SfileSendSuccess,
            this, [=](){
        ui->BsendFile->setEnabled(true);
        ui->LfilePath->clear();
    });


}

void model_client::on_BselectFile_clicked()
{
    QString filePath = QFileDialog::getOpenFileName();
    if (filePath.isEmpty())
    {
        QMessageBox::warning(this, "打开文件", "选择的文件路径不能为空");
        return;
    }
    ui->LfilePath->setText(filePath);
    ui->progressBar->setValue(0);
}

void model_client::on_BsendFile_clicked()
{
    emit SsendFile(ui->LfilePath->text());
    ui->BsendFile->setEnabled(false);
}

void model_client::on_BcancelSendFile_clicked()
{
    this->close();
}
