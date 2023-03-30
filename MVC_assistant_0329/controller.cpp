#include "controller.h"
#include "ui_controller.h"
#include <QThread>
#include <QFileDialog>
#include <QFileInfo>
#include <QFile>
#include <QTextBlock>
//#include <malloc.h>

controller::controller(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::controller)
{

    head = "$$$$^";

    timeFormat = "hh:mm:ss.zzz";


    time = "[" + QTime::currentTime().toString(timeFormat) + "] ";
    qDebug()<<"main thread"<<QThread::currentThread();


    SCmodel = new model(this);
    connect(SCmodel, &model::SgetClientMessage,
            this, &controller::outputClientMessage);


    Cmodel = new model_client;
    //when click sendFile button, initial sendfile class
    connect(this, &controller::SsendFile,
            Cmodel, &model_client::initialSendFile);
    connect(Cmodel, &model_client::SgetServerMessage,
            this, &controller::outputServerMessage);

    udpClient = new model_UDPconnnect;

    ui->setupUi(this);
}

controller::~controller()
{
    delete ui;
}

//---------------------------this should be view!!!!!!!!!!!-----------------------------
//output client message
void controller::outputClientMessage(QByteArray msg)
{
    if(!ui->CBhideDialog->isChecked()){
    if (ui->CBsendAddHead->isChecked() == false)
//not adding head
    {
        dialogHistory.append("[" + QTime::currentTime().toString(timeFormat) + "] "
                             + tr("Client:")+ msg.toHex() + "\n");
        if(ui->RBreceiveSetAsc->isChecked())
        //appear ASC
            ui->Edialog->append("[" + QTime::currentTime().toString(timeFormat) + "] " + tr("Client:") + QString(msg));
        else
        //apear HEX
            ui->Edialog->append("[" + QTime::currentTime().toString(timeFormat) + "] " + tr("Client:") + msg.toHex());
    }
    else{
//adding head
        if (QString(msg).startsWith("$$$$^")){
    //cutting head
            processedMsg = QString(msg).right(
                        QString(msg).length()-head.length());
            realMsg = processedMsg.section('|',1,1);
    //getting length of input
            lengthOfget = processedMsg.section('|',0,0).toInt();
        //-----------------------check integrity--------------------
            if (lengthOfget == realMsg.length())
            {
                //apear in ASCII
                if (ui->RBreceiveSetAsc->isChecked())
                    ui->Edialog->append("[" + QTime::currentTime().toString("hh:mm:ss.zzz") + "] " + tr("Client:") + realMsg.toUtf8() + "|" +
                                tr("信息长度") + QString::number(lengthOfget));
                else
                //apear in HEX
                    ui->Edialog->append("[" + QTime::currentTime().toString("hh:mm:ss.zzz") + "] " + tr("Client:") + realMsg.toUtf8().toHex() + "|" +
                                tr("信息长度") + QString::number(lengthOfget));
            }
            else
                ui->Edialog->append(tr("error!!!客户端信息遗失"));
        }
        else
            ui->Edialog->append(tr("error!!!客户端信息遗失"));
    }
    }
}

//output server message
void controller::outputServerMessage()
{
    if (!ui->CBhideDialog->isChecked() && QString(Cmodel->serverMessage) != ""){
    if (ui->CBsendAddHead->isChecked() == false)
//not adding head
    {
        dialogHistory.append("[" + QTime::currentTime().toString(timeFormat) + "] "
                + tr("Server:") + Cmodel->serverMessage.toHex() + "\n");
        if (ui->RBreceiveSetAsc->isChecked())
            ui->Edialog->append("[" + QTime::currentTime().toString(timeFormat) + "] "
                                + tr("Server:") + QString(Cmodel->serverMessage));
        else
            ui->Edialog->append("[" + QTime::currentTime().toString(timeFormat) + "] "
                                + tr("Server:") + Cmodel->serverMessage.toHex());
    }
    else{
//adding head
        if (QString(Cmodel->serverMessage).startsWith("$$$$^")){
            processedMsg = QString(Cmodel->serverMessage).right(
                        QString(Cmodel->serverMessage).length() - head.length());
            realMsg = processedMsg.section('|',1,1);
            lengthOfget = processedMsg.section('|',0,0).toInt();
            if (lengthOfget == realMsg.length())
            {
                //apear in ASCII
                if (ui->RBreceiveSetAsc->isChecked())
                    ui->Edialog->append("[" + QTime::currentTime().toString(timeFormat) + "] " + tr("Server:") + realMsg + "|" +
                                tr("信息长度") + QString::number(lengthOfget));
                else
                //apear in HEX
                    ui->Edialog->append("[" + QTime::currentTime().toString(timeFormat) + "] " + tr("Server:") + realMsg.toUtf8().toHex() + "|" +
                                tr("信息长度") + QString::number(lengthOfget));
            }
            else
                ui->Edialog->append(tr("error!!!服务器端信息遗失"));
        }
        else
            ui->Edialog->append(tr("error!!!服务器端信息遗失"));
    }
    }
}
//----------------------------------------------------------------------------------------

void controller::on_BnetSetCleanHostIP_clicked()
{
    ui->LElocalHostAdd->clear();
}

void controller::on_BnetSetPasteHostIP_clicked()
{
    ui->LElocalHostAdd->paste();
}

void controller::on_CBrecepSetSaveDialog_clicked()
{
    Qt::CaseSensitivity cs = ui->CBrecepSetSaveDialog->isChecked()?
                Qt::CaseSensitive:Qt::CaseInsensitive;
    ui->BrecepSetSaveDialog->setEnabled(cs == Qt::CaseSensitive);
}

void controller::on_BrecepSetSaveDialog_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(
                this,
                tr("Save File"),
                "",
                tr("Text File(*.txt)"));

    if (fileName != ""){
        QFile file(fileName);
        if(!file.open(QIODevice::WriteOnly)){
            QMessageBox::critical(this, tr("错误"),tr("无法写入该文件"));
        }
        else{
            QTextStream stream(&file);
            stream<<"THIS IS MAX DIALOG\n";
            stream<<dialogHistory;
            stream.flush();
            file.close();
        }
    }
}

void controller::on_LEsendSetrepPeriod_textChanged()
{
    ui->CBsendSetrepPeriod->setEnabled(!(ui->LEsendSetrepPeriod->text() == ""));
    if(ui->CBsendSetrepPeriod->isChecked())
        emit SsendRepeatTime(ui->LEsendSetrepPeriod->text().toInt());
}

//clean sendBox after sending
void controller::on_CBsendSetclearAftSend_clicked()
{
    Qt::CaseSensitivity cs = ui->CBsendSetclearAftSend->isChecked()?
                Qt::CaseSensitive : Qt::CaseInsensitive;

    if(cs == Qt::CaseSensitive)
        connect(ui->Bsend, SIGNAL(clicked(bool)),
                ui->EtextSend, SLOT(clear()));
    else
        disconnect(ui->Bsend, SIGNAL(clicked(bool)),
                   ui->EtextSend, SLOT(clear()));
}

//clean dialog
void controller::on_BcleanDialog_clicked()
{
    ui->Edialog->clear();
}

//-------------------------some works on HEX-----------------------------
void controller::on_EtextSend_textChanged()
{
//send in HEX
//STUPID: only type in 0-9 a-z A-Z, cannot distinguish HEX and ASCII
    if(ui->RBsendSelHEX->isChecked() == true)
    {
        QTextEdit *mTextEdit = ui->EtextSend;
        QTextCursor mTextCursor = mTextEdit->textCursor();
//position of cursor
        int CurLineNum=mTextEdit->textCursor().blockNumber();
        int mPosInBlock=mTextCursor.positionInBlock();
//current line
        QString CurLineText = mTextEdit->document()->findBlockByLineNumber(CurLineNum).text();
        char inputChar = 0;
        if (CurLineText == "")
            return;
        else
            inputChar = CurLineText.at(mPosInBlock-1).toLatin1();
        //input must 0-9/a-f/A-F
        if ((inputChar == ' ') ||
                (inputChar >= '0' && inputChar <='9') ||
                (inputChar >= 'a' && inputChar <='f') ||
                (inputChar >= 'A' && inputChar <='F'))
        {
            //ui->Bsend->setEnabled(!(ui->EtextSend->toPlainText() == ""));
            return;
        }
        else
        {
            mTextEdit->textCursor().deletePreviousChar();
            mTextEdit->setTextCursor(mTextEdit->textCursor());
        }
    }
}

//clean send-Button
void controller::on_BcleanSend_clicked()
{
    ui->EtextSend->clear();
}

//activate listen
void controller::on_BactivateListen_clicked()
{

    /*SCmodel = new model(this);
    connect(SCmodel, &model::SgetClientMessage,
            this, &controller::outputClientMessage);*/
    connect(SCmodel, &model::SstartListening,
            this, [=](){
        ui->BactivateListen->setEnabled(false);
        ui->BactivateListen->setText(tr("监听已启动"));
        ui->CBtypePro->setEnabled(false);
        ui->Edialog->append("[" + QTime::currentTime().toString(timeFormat) + "] " + "TCPServer start listening");
    });
    SCmodel->activateListen();

    connect(SCmodel, &model::SsocketListUpdate,
            this, [=](QList<QTcpSocket*> socketList){
        ui->CBclientList->clear();
        if(socketList.size() != 0){
            ui->CBclientList->addItem(tr("所有客户端共") +
                                      QString::number(socketList.size()) + tr("台"));
            for(int j = 0;
                j < socketList.size();
                j++)
            {
                ui->CBclientList->addItem(socketList.at(j)->peerAddress().toString().section(':', -1, -1) + " " +
                                          QString::number(socketList.at(j)->peerPort()) + " " +
                                          QString::number(socketList.at(j)->socketDescriptor()));
            }
        }
        else
            ui->CBclientList->addItem(tr("未有客户端连接"));
    });
}

//when clicking 连接button
void controller::on_BsendSetConnect_clicked()
{
    /*Cmodel = new model_client;
    //when click sendFile button, initial sendfile class
    connect(this, &controller::SsendFile,
            Cmodel, &model_client::initialSendFile);
    connect(Cmodel, &model_client::SgetServerMessage,
            this, &controller::outputServerMessage);*/
    Cmodel->client();
    Cmodel->m_c->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, 1024*1024);
    //qDebug()<<"client Socket send buffer size"<<Cmodel->m_c->socketOption(QAbstractSocket::SendBufferSizeSocketOption);


    connect(Cmodel->m_c, &QTcpSocket::connected,
            this, [=](){
        ui->Edialog->append("[" + QTime::currentTime().toString(timeFormat) + "] " +
                            tr("TCP Client:connect to the server"));
        qDebug()<<Cmodel->m_c->socketDescriptor();
        ui->BsendSetBreak->setEnabled(true);
        ui->BsendSetConnect->setEnabled(false);
        ui->LElocalHostAdd->setEnabled(false);
        ui->LElocalHostPort->setEnabled(false);
    });
    connect(Cmodel->m_c, &QTcpSocket::disconnected,
            this, [=](){
        ui->Edialog->append("[" + QTime::currentTime().toString(timeFormat) + "] " +
                            tr("TCP Client:disconnect from the server"));
        ui->BsendSetConnect->setEnabled(true);
        ui->BsendSetBreak->setEnabled(false);
        ui->LElocalHostAdd->setEnabled(true);
        ui->LElocalHostPort->setEnabled(true);
    });
}

//disconnection
void controller::on_BsendSetBreak_clicked()
{
    Cmodel->serverDisconnected();
    emit SclientDisconnectFromServer();
}

//send File
void controller::on_BsendFile_clicked()
{
//only from client to server
    if(Cmodel->m_c->state() != QAbstractSocket::ConnectedState)
    {
        QMessageBox::warning(this, "WARNING", "请先连接服务器");
        return;
    }
    ui->RBsendZoneClient->setChecked(true);
    emit SsendFile();
    Cmodel->show();
}


//when changing type of input
//named wrong
//changing QString(HEX) to QByteArray(HEX)
void controller::changeHexToAsc(QString str, QByteArray &senddata)
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

char controller::convertHexChart(char ch)
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

//send Esend content in the style of select,
//include from server and client,
//selection of client
//add or dont add head,
//using TCP or UDP,
//using HEX or ASCII
void controller::sendMessage()
{
    //sending initialize
    QString tempInput = ui->EtextSend->toPlainText();
    if(ui->RBsendSelHEX->isChecked())
    {
        QByteArray tempData;
        changeHexToAsc(tempInput.simplified(), tempData);
        tempInput = QString(tempData);
    }
    qDebug()<<"send message"<<tempInput;

    //start sending
    if(ui->CBtypePro->currentText() == "TCP"){//send by TCP
        if(ui->RBsendZoneClient->isChecked())
        {
        //send from client
            //check connect state
            if(Cmodel->m_c->state() == QAbstractSocket::ConnectedState)
            {
                if (ui->CBsendAddHead->isChecked() == false){
                //not adding head
                    Cmodel->msg = tempInput.toUtf8();
                    Cmodel->sendClientMessage();
                }
                else{
                //adding head to text
                    QString temp = head + QString::number(tempInput.length())
                            + "|" + tempInput;
                    Cmodel->msg = temp.toUtf8();
                    Cmodel->sendClientMessage();
                }
            }
            else
            {
                QMessageBox::warning(this, "WARNING", "请先连接服务器");
                return;
            }
        }
        else
        {
        //sending from server
            if (ui->CBsendAddHead->isChecked() == false)
            {
            //no head
                SCmodel->msg = tempInput.toUtf8();
                SCmodel->sendServerMessage(ui->CBclientList->currentIndex());
            }
            else
            //add head head int(size) text
            {
                QString temp = head + QString::number(tempInput.length())
                        + "|" + tempInput;
                SCmodel->msg = temp.toUtf8();
                SCmodel->sendServerMessage(ui->CBclientList->currentIndex());
            }
        }
    }
    else{//send by UDP
        if(ui->CBsendAddHead->isChecked() == false){
            emit SsendUdpMsg(tempInput);
            ui->Edialog->append(tr("send Message by UDP：") + tempInput);
        }
        else{
            QString temp = head + QString::number(tempInput.length())
                    + "|" + tempInput;
            emit(SsendUdpMsg(temp));
            ui->Edialog->append(tr("send Messsage by UDP：") + temp);
        }

    }
}

//changing to ASCII input
void controller::on_RBsendSelASC_toggled()
{
    if ((ui->EtextSend->toPlainText() !="") &&
            (ui->RBsendSelASC->isChecked())){
        QString temp = ui->EtextSend->toPlainText().simplified();
        QByteArray data;
        changeHexToAsc(temp, data);
        ui->EtextSend->clear();
        ui->EtextSend->append(QString(data));
    }
}

//Changing to HEX input
void controller::on_RBsendSelHEX_toggled()
{
    if ((ui->EtextSend->toPlainText() !="") && (ui->RBsendSelHEX->isChecked())){
        userInput = ui->EtextSend->toPlainText().toUtf8();
        ui->EtextSend->clear();
        ui->EtextSend->append(userInput.toHex());
    }
}

//click sendButton
void controller::on_Bsend_clicked()
{
    sendMessage();
}

//change ui appearance
void controller::on_RBsendZoneClient_toggled()
{
    if(ui->RBsendZoneClient->isChecked())
    {
        ui->Lip->setText(tr("远程主机地址"));
        ui->Lport->setText(tr("远程主机端口"));
    }
}

void controller::on_RBsendZoneServer_toggled()
{
    if(ui->RBsendZoneServer->isChecked())
    {
        ui->Lip->setText(tr("本地主机地址"));
        ui->Lport->setText(tr("本地主机端口"));
    }

}

//send message on time
void controller::on_BsendOnTime_clicked()
{
    SendTextOnTime = new sendOnTime(this);
    connect(SendTextOnTime, &sendOnTime::SstartCaculateTime,
            this, [=](){
        ui->BsendOnTime->setEnabled(false);
    });

    connect(SendTextOnTime, &sendOnTime::SshowWhatGoingToSend,
            this, [=](){

    });



    connect(SendTextOnTime, &sendOnTime::SsendMessageOnTime,
            this, [=](QString msg, int sel)
    {
        if (sel == 1)
        {
        //send from client
            if(Cmodel->m_c->state() != QAbstractSocket::ConnectedState)
            {
                QMessageBox::warning(this, "WARNING", "请先连接服务器");
                return;
            }
            if (ui->CBsendAddHead->isChecked() == false){
            //not adding head
                Cmodel->msg = msg.toUtf8();
                Cmodel->sendClientMessage();
            }
            else{
            //adding head to text
                QString temp = head + QString::number(ui->EtextSend->toPlainText().length())
                        + "|" + msg;
                Cmodel->msg = temp.toUtf8();
                Cmodel->sendClientMessage();
            }
        }
        else
        {
        //send from server
        //sending from server
            if(Cmodel->m_c->state() != QAbstractSocket::ConnectedState)
            {
                QMessageBox::warning(this, "WARNING", "请先连接客户端");
                return;
            }
            if (ui->CBsendAddHead->isChecked() == false)
            {
            //no head
                SCmodel->msg = msg.toUtf8();
                SCmodel->sendServerMessage(ui->CBclientList->currentIndex());
            }
            else
            //add head head int(size) text
            {
                QString temp = head + QString::number(ui->EtextSend->toPlainText().length())
                        + "|" + msg;
                SCmodel->msg = temp.toUtf8();
                SCmodel->sendServerMessage(ui->CBclientList->currentIndex());
            }
        }
        ui->BsendOnTime->setEnabled(true);
    });

    SendTextOnTime->show();
}

//send message repeatly
void controller::on_CBsendSetrepPeriod_clicked()
{
    if (ui->CBsendSetrepPeriod->isChecked())
    {
        repeatSend = new repeatSendMsg(this);

        //get time interval
        connect(this, &controller::SsendRepeatTime,
                repeatSend, &repeatSendMsg::getRepeatTime);
        emit SsendRepeatTime(ui->LEsendSetrepPeriod->text().toInt());

        //time out send msg
        connect(repeatSend->TrepeatSendMsg, &QTimer::timeout,
                this, &controller::sendRepeatMsg);
    }
    else
    {
        disconnect(this, &controller::SsendRepeatTime,
                   repeatSend, &repeatSendMsg::getRepeatTime);
        disconnect(repeatSend->TrepeatSendMsg, &QTimer::timeout,
                this, &controller::sendRepeatMsg);
    }
}

repeatSendMsg::repeatSendMsg(QWidget *parent) : QObject(parent)
{
    TrepeatSendMsg = new QTimer;
}

void repeatSendMsg::getRepeatTime(int time)
{
    repeatTime = time;
    TrepeatSendMsg->start(repeatTime);
}

void controller::sendRepeatMsg()
{
    sendMessage();
}

//search in dialog
void controller::on_Bsearch_clicked()
{
    QString initialDialog = ui->Edialog->toPlainText();
    QTextCursor cursor = ui->Edialog->textCursor();
    QTextCharFormat initialFormat;
    initialFormat.setBackground(Qt::white);
    initialFormat.setForeground(Qt::black);

    cursor.setPosition(0, QTextCursor::MoveAnchor);
    cursor.setPosition(initialDialog.size(), QTextCursor::KeepAnchor);

    cursor.mergeCharFormat(initialFormat);
    ui->Edialog->setTextCursor(cursor);

    if(ui->LEsearch->text().isEmpty()){
        QMessageBox::warning(this, "WARNING", tr("请输入要查找的内容"));
        return;
    }
    else
    {
        QString searchLine = ui->LEsearch->text();
        int startIndex[1024*4] = {-1};
        int endIndex[1024*4] = {-1};
        endIndex[5]=10;
        int num = 1;


        do
        {
            QString dialog = ui->Edialog->toPlainText();
            //if(num != 1)
                //dialog = dialog.mid(endIndex[num-2]);
            if(num != 1)
                startIndex[num - 1] = dialog.indexOf(searchLine,endIndex[num - 2]);
            else
                startIndex[num - 1] = dialog.indexOf(searchLine);
            endIndex[num - 1] = startIndex[num - 1] + searchLine.size();
            num++;
        }while(startIndex[num-2] != -1);



        int t = 0;
        while(t < num - 2)
        {
            qDebug()<<"t "<<t;
            cursor.setPosition(startIndex[t], QTextCursor::MoveAnchor);
            cursor.setPosition(endIndex[t], QTextCursor::KeepAnchor);

            QTextCharFormat format;
            format.setBackground(Qt::green);
            format.setForeground(Qt::yellow);

            cursor.mergeCharFormat(format);

            ui->Edialog->setTextCursor(cursor);
            t++;
        }

        ui->Edialog->setFocus();
    }
}

//clear all search
void controller::on_BclearSearch_clicked()
{
    QString initialDialog = ui->Edialog->toPlainText();
    QTextCursor cursor = ui->Edialog->textCursor();
    QTextCharFormat initialFormat;
    initialFormat.setBackground(Qt::white);
    initialFormat.setForeground(Qt::black);

    cursor.setPosition(0, QTextCursor::MoveAnchor);
    cursor.setPosition(initialDialog.size(), QTextCursor::KeepAnchor);

    cursor.mergeCharFormat(initialFormat);
    ui->Edialog->setTextCursor(cursor);
}

//--------------------------TCP-server initialization uncomplete--------------------
void controller::on_BserverInitial_clicked()
{
}

//input host and ip
void controller::on_LElocalHostAdd_textChanged(const QString &arg1)
{
    if(ui->CBtypePro->currentText() == "TCP")
        Cmodel->ip = arg1;
    else
        udpClient->ip = arg1;
}

void controller::on_LElocalHostPort_textChanged(const QString &arg1)
{
    if(ui->CBtypePro->currentText() == "TCP"){
        SCmodel->port = arg1.toInt();
        Cmodel->port = arg1.toInt();
    }
    else
        udpClient->port = arg1.toInt();
}

//change type of protocal between UDP and TCP
void controller::on_CBtypePro_currentIndexChanged(const QString &arg1)
{
    if(arg1 == "UDP")
    {
        ui->BactivateListen->setEnabled(false);
        ui->LElocalHostAdd->setText(tr("192.168.1.25"));
        ui->BsendSetBreak->setEnabled(true);
        ui->CBclientList->setEnabled(false);
        ui->LEcastSelect->setEnabled(false);

        connect(this, &controller::SsendUdpMsg,
                udpClient, &model_UDPconnnect::sendMsg);
        connect(udpClient, &model_UDPconnnect::getUdpMsg,
                this, &controller::getUPDMsg);

        disconnect(SCmodel, &model::SgetClientMessage,
                this, &controller::outputClientMessage);
        disconnect(Cmodel, &model_client::SgetServerMessage,
                this, &controller::outputServerMessage);
        disconnect(this, &controller::SsendFile,
                Cmodel, &model_client::initialSendFile);
    }
    else
    {
        ui->BactivateListen->setEnabled(true);
        ui->LElocalHostAdd->setText(tr("127.0.0.1"));
        ui->CBclientList->setEnabled(true);
        ui->LEcastSelect->setEnabled(true);

        connect(SCmodel, &model::SgetClientMessage,
                this, &controller::outputClientMessage);
        connect(Cmodel, &model_client::SgetServerMessage,
                this, &controller::outputServerMessage);
        connect(this, &controller::SsendFile,
                Cmodel, &model_client::initialSendFile);
        disconnect(this, &controller::SsendUdpMsg,
                   udpClient, &model_UDPconnnect::sendMsg);
        disconnect(udpClient, &model_UDPconnnect::getUdpMsg,
                this, &controller::getUPDMsg);
    }
}

void controller::getUPDMsg(QString str)
{
    if (ui->CBsendAddHead->isChecked() == false)
//not adding head
    {
        if(ui->RBreceiveSetAsc->isChecked())
        //appear ASC
            ui->Edialog->append("[" + QTime::currentTime().toString(timeFormat) + "] " + tr("获得UDP传输：") + str);
        else
        //apear HEX
            ui->Edialog->append("[" + QTime::currentTime().toString(timeFormat) + "] " + tr("获得UDP传输：") + str.toLatin1().toHex());
    }
    else{
//adding head
        if (str.startsWith("$$$$^")){
    //cutting head
            processedMsg = str.right(
                        str.length()-head.length());
            realMsg = processedMsg.section('|',1,1);
    //getting length of input
            lengthOfget = processedMsg.section('|',0,0).toInt();
        //-----------------------check integrity--------------------
            if (lengthOfget == realMsg.length())
            {
                //apear in ASCII
                if (ui->RBreceiveSetAsc->isChecked())
                    ui->Edialog->append("[" + QTime::currentTime().toString(timeFormat) + "] " + tr("获得UDP传输：") + realMsg.toUtf8() + " " +
                                tr("信息长度") + QString::number(lengthOfget));
                else
                //apear in HEX
                    ui->Edialog->append("[" + QTime::currentTime().toString(timeFormat) + "] " + tr("获得UDP传输：") + realMsg.toUtf8().toHex() + " " +
                                tr("信息长度") + QString::number(lengthOfget));
            }
            else
                ui->Edialog->append(tr("error!!!获得UDP传输信息遗失"));
        }
        else
            ui->Edialog->append(tr("error!!!获得UDP传输信息遗失"));
    }
}

//repeat Dialog
void controller::on_BsendByDialog_clicked()
{
    sendDialog = new sendByDialog(this);
    connect(sendDialog, &sendByDialog::sendInformation,
            this, &controller::repeatDialog);
    connect(sendDialog, &sendByDialog::SopenRightDialog,
            this,[=](QTime time){
        ui->Edialog->append("[" + QTime::currentTime().toString(timeFormat) + "] " +
                            tr("Going to repeat dialog starting at ") + '[' + time.toString(timeFormat) + ']');
    });
    connect(sendDialog, &sendByDialog::SrepeatComplete,
            this,[=](){
        ui->Edialog->append("[" + QTime::currentTime().toString(timeFormat) + "] " +
                            tr("Dialog read Complete"));
    });
    connect(sendDialog, &sendByDialog::SopenWrongDialog,
            this, &controller::repeatNotMyDialog, Qt::QueuedConnection);
    connect(sendDialog, &sendByDialog::SendTask,
            this, [=](){
        sendDialog->close();
        disconnect(sendDialog,&sendByDialog::sendInformation,
                   this, &controller::repeatDialog);
        disconnect(sendDialog->dialogRead, &readDialog::warningNotMyDialog,
                   this, &controller::repeatNotMyDialog);
    });
    sendDialog->show();
}

void controller::repeatDialog(QString side, QString content)
{

    if(side == " Server" )
    {//send from server
        if (ui->CBsendAddHead->isChecked() == false)
        {
        //no head
            SCmodel->msg = content.toUtf8();
            SCmodel->sendServerMessage(ui->CBclientList->currentIndex());
            qDebug()<<"going to send dialog"<<content;
        }
        else
        //add head head int(size) text
        {
            QString temp = head + QString::number(content.length())
                    + "|" + content;
            SCmodel->msg = temp.toUtf8();
            SCmodel->sendServerMessage(ui->CBclientList->currentIndex());
            qDebug()<<"Text";
        }
    }
    else if(side == " Client" && Cmodel->m_c->state() == QAbstractSocket::ConnectedState)
    {
        if (ui->CBsendAddHead->isChecked() == false){
        //not adding head
            Cmodel->msg = content.toUtf8();
            Cmodel->sendClientMessage();
            qDebug()<<"going to send dialog"<<content;
        }
        else{
        //adding head to text
            QString temp = head + QString::number(content.length())
                    + "|" + content;
            Cmodel->msg = temp.toUtf8();
            Cmodel->sendClientMessage();
        }
    }
}

void controller::repeatNotMyDialog()
{
    QMessageBox::warning(this, "WARNING!!!", tr("这不是我的日志，请打开我的日志！"));
    sendDialog->close();
    sendDialog->deleteLater();
    disconnect(sendDialog,&sendByDialog::sendInformation,
               this, &controller::repeatDialog);
    disconnect(sendDialog->dialogRead, &readDialog::warningNotMyDialog,
               this, &controller::repeatNotMyDialog);
}
