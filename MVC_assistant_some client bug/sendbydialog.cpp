#include "sendbydialog.h"
#include "ui_sendbydialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QThread>
#include <QDebug>

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
    ui->LEdialogPath->setText(filePath);
    ui->progressBar->setValue(0);
}

void sendByDialog::on_Bconfirm_clicked()
{
    dialogRead = new readDialog;
    QThread* thread = new QThread(this);

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
            this, [=](QTime time){
        emit SopenRightDialog(time);
    });
    connect(dialogRead, &readDialog::SreadComplete,
            this, [=](){
        emit SrepeatComplete();
    });


    dialogRead->moveToThread(thread);
    thread->start();

    QTime time = QTime::currentTime();

    emit sendFile(ui->LEdialogPath->text(), time);
    ui->Bconfirm->setEnabled(false);
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

void readDialog::readAndSendDialog(QString path, QTime startSendTime)
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

    QTime dialogLastTime;
    QTime dialogNextTime;
    QTime lastSendTime;

    QString content;
    QString side;
    QString contentIncludeSide;
    int isFirstContent = 0;

    while(!file.atEnd())
    {
        QByteArray line = file.readLine();
        QString lineASC = QString(line);
        QString dialogTimeString = lineASC.section('[',1,-1).section(']',0,0);
        dialogNextTime = QTime::fromString(dialogTimeString, "hh:mm:ss.zzz");
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
        else if(contentIncludeSide.startsWith(" Server") ||
                contentIncludeSide.startsWith(" Client"))
        {
            side = lineASC.section(']', 1, 1).section(":", 0, 0);
            content = lineASC.section(':', 3, -1).simplified();
            QByteArray tempData;
            changeHexToAsc(content, tempData);
           if (isFirstContent == 0)
           {
               emit SopenMyDialog(dialogNextTime);
               emit sendInformation(side, QString(tempData));
               qDebug()<<"send next message:"<<content
                      <<"from"<<side
                      <<"at"<<QTime::currentTime();
               lastSendTime = QTime::currentTime();
               dialogLastTime = dialogNextTime;
               lineNum++;
               isFirstContent = 1;
           }
           else
           {
               int timeDifference = dialogLastTime.msecsTo(dialogNextTime);

               while(1)
               {
                   if(lastSendTime.msecsTo(QTime::currentTime()) >= timeDifference)
                   {
                       emit sendInformation(side, QString(tempData));
                       lastSendTime = QTime::currentTime();
                       dialogLastTime = dialogNextTime;
                       qDebug()<<"send next message:"<<QString(tempData)
                              <<"from"<<side
                              <<"at"<<QTime::currentTime();
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

    if(sendSize == fileSize)
        emit SreadComplete();
}
