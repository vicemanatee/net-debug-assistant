#include "sendbydialog.h"
#include "ui_sendbydialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QThread>
#include <QDebug>
#include <QtCore/qmath.h>

sendByDialog::sendByDialog(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::sendByDialog)
{
    ui->setupUi(this);
}

sendByDialog::~sendByDialog()
{
    delete ui;
}

void sendByDialog::on_BselectDialogPath_clicked()
{
    QString filePath = QFileDialog::getOpenFileName();
    if (filePath.isEmpty())
    {
        QMessageBox::warning(this, tr("打开文件"), tr("选择文件为空"));
        return;
    }
    ui->Bconfirm->setEnabled(true);
    ui->groupBox->setEnabled(true);
    ui->CBrepeadSpead->setEnabled(true);
    ui->LEdialogPath->setText(filePath);
    ui->progressBar->setValue(0);
}

void sendByDialog::on_Bconfirm_clicked()
{
    dialogRead = new readDialog;
    QThread* thread = new QThread(this);

    //readBaseSide = 0 read from receive
    //readBaseSide = 1 read from send
    if (ui->RBsendBaseOnReceive->isChecked())
        readBaseSide = 0;
    else
        readBaseSide = 1;
    connect(this, &sendByDialog::sendFile,
            dialogRead, &readDialog::readAndSendDialog);
    connect(dialogRead, &readDialog::sendProgress,
            ui->progressBar, &QProgressBar::setValue);
    connect(dialogRead, &readDialog::sendInformation,
            this, [=](QString side, QString content){
        content = content.simplified();
        emit sendInformation(side, content);
    });
    connect(this, &sendByDialog::SendTask,
            this, [=](){
        thread->terminate();
    });
    connect(dialogRead, &readDialog::warningNotMyDialog,
            this, [=](){
        emit SopenWrongDialog();
        emit SendTask();
    });
    connect(dialogRead, &readDialog::SopenMyDialog,
            this, [=](QDateTime time){
        emit SopenRightDialog(time);
    });
    connect(dialogRead, &readDialog::SreadComplete,
            this, [=](){
        emit SrepeatComplete();
    });


    dialogRead->moveToThread(thread);
    thread->start();

    QDateTime time = QDateTime::currentDateTime();

    int spead = qPow(2, ui->CBrepeadSpead->currentIndex());
    emit sendFile(ui->LEdialogPath->text(), time, readBaseSide, spead);
    ui->Bconfirm->setEnabled(false);
    ui->groupBox->setEnabled(false);
    ui->CBrepeadSpead->setEnabled(false);
}

void sendByDialog::on_Bcancel_clicked()
{
    emit SendTask();
}

void readDialog::changeHexToAsc(QString str, QByteArray &senddata)
{
    int hexdata,lowhexdata;
    int hexdatalen = 0;
    int len = str.length();
    senddata.resize(len/2);
    char lstr,hstr;
    for(int i=0; i<len; )
    {
        hstr=str[i].toLatin1();
        if(hstr == ' ')
        {
            i++;
            continue;
        }
        i++;
        if(i >= len)
            break;
        lstr = str[i].toLatin1();
        hexdata = convertHexChart(hstr);
        lowhexdata = convertHexChart(lstr);
        if((hexdata == 16) || (lowhexdata == 16))
            break;
        else
            hexdata = hexdata*16+lowhexdata;
        i++;
        senddata[hexdatalen] = (char)hexdata;
        hexdatalen++;
    }
    senddata.resize(hexdatalen);
}

int readDialog::convertHexChart(char ch)
{
    if((ch >= '0') && (ch <= '9'))
        return ch-0x30;  // 0x30 对应 ‘0’
    else if((ch >= 'A') && (ch <= 'F'))
        return ch-'A'+10;
    else if((ch >= 'a') && (ch <= 'f'))
        return ch-'a'+10;
    else
        return ch-ch;//不在0-f范围内的会发送成0
}



//get dialog sending time, side, and content
//caculate the interval time and emit signal when the right time is coming
//including content and client or server mode
//signals: sendInformation(QString side, QSting content);
readDialog::readDialog(QWidget *parent) :
    QObject(parent)
{

}

void readDialog::readAndSendDialog(QString path, QDateTime startSendTime, int readBaseSide, int spead)
{
    qDebug()<<"open dialog in Thread: "<<
              QThread::currentThread();
    QFile file(path);
    QFileInfo info(path);
    int fileSize = info.size();
    int sendSize = 0;
    int sendDialogProgress = 0;
    int lineNum = 0;
    file.open(QFile::ReadOnly);

    QDateTime dialogLastTime;
    QDateTime dialogNextTime;
    QDateTime lastSendTime;

    QString content;
    QString side;
    QString contentIncludeSide;
    int isFirstContent = 0;

    while(!file.atEnd())
    {
        QByteArray line = file.readLine();
        QString lineASC = QString(line);
        QString dialogTimeString = lineASC.section('[',1,-1).section(']',0,0);
        dialogNextTime = QDateTime::fromString(dialogTimeString, "yyyy.mm.dd hh:mm:ss.zzz");
        qDebug()<<"get dialog next time:"<<dialogNextTime;
        contentIncludeSide = lineASC.section(']', 1, -1);


        if(lineNum == 0){
            if(lineASC != tr("THIS IS MAX DIALOG\n")){
                emit warningNotMyDialog();
                break;
            }
            else{
                lineNum++;
            }
        }
        if(readBaseSide == 0)
        {
            //read base on receive
            if(contentIncludeSide.startsWith(" Receive Server") ||
                    contentIncludeSide.startsWith(" Receive Client") ||
                    contentIncludeSide.startsWith(" Receive UDP"))

            {
                side = lineASC.section(']', 1, 1).section(":", 0, 0);
                if (side.endsWith("Client Message"))
                    side = " Client";
                else if (side.endsWith("Server Message"))
                    side = " Server";
                else
                    side = " UDP";
                content = lineASC.section(':', 3, -1).simplified();
                QByteArray tempData;
                changeHexToAsc(content, tempData);
                if (isFirstContent == 0)
                {
                    emit SopenMyDialog(dialogNextTime);
                    emit sendInformation(side, QString(tempData));
                    qDebug()<<"send next message:"<<content
                            <<"from"<<side
                            <<"at"<<QDateTime::currentDateTime();
                    lastSendTime = QDateTime::currentDateTime();
                    dialogLastTime = dialogNextTime;
                    lineNum++;
                    isFirstContent = 1;
                }
               else
               {
                   int timeDifference = dialogLastTime.msecsTo(dialogNextTime);
                   timeDifference = timeDifference / spead;

                   while(1)
                   {
                       if(lastSendTime.msecsTo(QDateTime::currentDateTime()) >= timeDifference)
                       {
                           emit sendInformation(side, QString(tempData));
                           lastSendTime = QDateTime::currentDateTime();
                           dialogLastTime = dialogNextTime;
                           qDebug()<<"send next message:"<<QString(tempData)
                                  <<"from"<<side
                                  <<"at"<<QDateTime::currentDateTime();
                           lineNum++;
                           break;
                       }
                   }
               }
            }

            sendSize += line.size();
            sendDialogProgress = sendSize*100/fileSize;
            emit sendProgress(sendDialogProgress);
        }
        else
        //read base on send
        {
            if(contentIncludeSide.startsWith(" Send by"))
            {
                side = lineASC.section(']', 1, 1).section(":", 0, 0);
                if (side.endsWith("Client"))
                    side = " Client";
                else if (side.endsWith("Server"))
                    side = " Server";
                else
                    side = " UDP";
                content = lineASC.section(':', 3, -1).simplified();
                QByteArray tempData;
                changeHexToAsc(content, tempData);
               if (isFirstContent == 0)
               {
                   emit SopenMyDialog(dialogNextTime);
                   emit sendInformation(side, QString(tempData));
                   qDebug()<<"send next message:"<<content
                          <<"from"<<side
                          <<"at"<<QDateTime::currentDateTime();
                   lastSendTime = QDateTime::currentDateTime();
                   dialogLastTime = dialogNextTime;
                   lineNum++;
                   isFirstContent = 1;
               }
               else
               {
                   int timeDifference = dialogLastTime.msecsTo(dialogNextTime);
                   timeDifference = timeDifference / spead;

                   while(1)
                   {
                       if(lastSendTime.msecsTo(QDateTime::currentDateTime()) >= timeDifference)
                       {
                           emit sendInformation(side, QString(tempData));
                           lastSendTime = QDateTime::currentDateTime();
                           dialogLastTime = dialogNextTime;
                           qDebug()<<"send next message:"<<QString(tempData)
                                  <<"from"<<side
                                  <<"at"<<QDateTime::currentDateTime();
                           lineNum++;
                           break;
                       }
                   }
               }
            }
            sendSize += line.size();
            sendDialogProgress = sendSize*100/fileSize;
            emit sendProgress(sendDialogProgress);
        }
    }

    if(sendSize == fileSize)
        emit SreadComplete();
}
