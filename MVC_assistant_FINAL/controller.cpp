#include "controller.h"
#include "ui_controller.h"
#include <QThread>
#include <QFileDialog>
#include <QFileInfo>
#include <QFile>
#include <QTextBlock>
#include <QDateTime>

controller::controller(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::controller)
{
    //recordState = 0: not recording
    //recordState = 1: recording
    recordState = 0;

    head = "$$$$^";

    timeFormat = "yyyy.mm.dd hh:mm:ss.zzz";

    qDebug()<<"main thread"<<QThread::currentThread();

//------------------------------SERVER MODEL----------------------------------------
    SCmodel = new model(this);
/*----------server subthread----------------
    QThread* serverThread = new QThread;
    SCmodel->moveToThread(serverThread);
    serverThread->start();*/

    //start listening
    connect(this, &controller::STcpServerStartListening,
            SCmodel, &model::activateListen);
    //send server message
    connect(this, &controller::SsendServerMsg,
            SCmodel, &model::slotSendServerMessage);
    //get client message
    connect(SCmodel, &model::SgetClientMessage,
            this, &controller::outputClientMessage);
    //server close
    connect(this, &controller::STcpServerclose,
            SCmodel, &model::serverClose);
    //changes on view when server start listening
    connect(SCmodel, &model::SstartListening,
            this, &controller::slotViewChangeTcpServerStartListening);
    //changes on view when server close
    connect(SCmodel, &model::SserverClose,
            this, &controller::slotViewChangeTcpServerClose);

//-------------------------------CLIENT MODEL----------------------------------------
    Cmodel = new model_client;
/*-----------client subthread---------------
    QThread* clientThread = new QThread;
    Cmodel->moveToThread(clientThread);
    clientThread->start();*/

    //connect to server
    connect(this, &controller::STcpClientConnectStartConnect,
            Cmodel, &model_client::client);
    //send client message
    connect(this, &controller::SsendClientMsg,
            Cmodel, &model_client::slotSendClientMessage);
    //get server message
    connect(Cmodel, &model_client::SgetServerMessage,
            this, &controller::outputServerMessage);
    //when click sendFile button, initial sendfile class
    connect(this, &controller::SsendFile,
            Cmodel, &model_client::initialSendFile);
    //disconnect from server
    connect(this, &controller::SclientDisconnectFromServer,
            Cmodel, &model_client::serverDisconnected);
    //change on view when client connect successful
    connect(Cmodel->m_c, &QTcpSocket::connected,
            this, &controller::slotTCPClientConnectSuccess);
    //change on view when client disconnect
    connect(Cmodel->m_c, &QTcpSocket::disconnected,
            this, &controller::slotTCPClientDisconnect);

//---------------------------------UDP MODEL--------------------------------------
    udpClient = new model_UDPconnnect;

    ui->setupUi(this);

//---------------------------------disable function--------------------------------
    //send on time button
    ui->BsendOnTime->setVisible(false);
    //add head function
    ui->CBsendAddHead->setVisible(false);
}

controller::~controller()
{
    delete ui;
}

//output client message
void controller::outputClientMessage(QByteArray msg)
{
    if(!ui->CBhideDialog->isChecked()){
    if (ui->CBsendAddHead->isChecked() == false)
//not adding head
    {
        dialogHistory.append("[" + QDateTime::currentDateTime().toString(timeFormat) + "] "
                             + tr("Receive Client Message:")+ msg.toHex() + "\n");
        emit SdialogSaveUpdate("[" + QDateTime::currentDateTime().toString(timeFormat) + "] "
                               + tr("Receive Client Message:")+ msg.toHex() + "\n");
        if(ui->RBreceiveSetAsc->isChecked())
        //appear ASC
            ui->Edialog->append("[" + QDateTime::currentDateTime().toString(timeFormat) + "] " +
                                tr("Receive Client Message: ") + QString(msg));
        else
        //apear HEX
            ui->Edialog->append("[" + QDateTime::currentDateTime().toString(timeFormat) + "] " +
                                tr("Receive Client Message: ") + msg.toHex());
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
                    ui->Edialog->append("[" + QDateTime::currentDateTime().toString("hh:mm:ss.zzz") + "] " + tr("Client:") + realMsg.toUtf8() + "|" +
                                tr("信息长度") + QString::number(lengthOfget));
                else
                //apear in HEX
                    ui->Edialog->append("[" + QDateTime::currentDateTime().toString("hh:mm:ss.zzz") + "] " + tr("Client:") + realMsg.toUtf8().toHex() + "|" +
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
        dialogHistory.append("[" + QDateTime::currentDateTime().toString(timeFormat) + "] "
                + tr("Receive Server Message:") + Cmodel->serverMessage.toHex() + "\n");
        emit SdialogSaveUpdate("[" + QDateTime::currentDateTime().toString(timeFormat) + "] "
                               + tr("Receive Server Message:") + Cmodel->serverMessage.toHex() + "\n");
        if (ui->RBreceiveSetAsc->isChecked())
            ui->Edialog->append("[" + QDateTime::currentDateTime().toString(timeFormat) + "] "
                                + tr("Receive Server Message: ") + QString(Cmodel->serverMessage));
        else
            ui->Edialog->append("[" + QDateTime::currentDateTime().toString(timeFormat) + "] "
                                + tr("Receive Server Message: ") + Cmodel->serverMessage.toHex());
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
                    ui->Edialog->append("[" + QDateTime::currentDateTime().toString(timeFormat) + "] " + tr("Server:") + realMsg + "|" +
                                tr("信息长度") + QString::number(lengthOfget));
                else
                //apear in HEX
                    ui->Edialog->append("[" + QDateTime::currentDateTime().toString(timeFormat) + "] " + tr("Server:") + realMsg.toUtf8().toHex() + "|" +
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

//save dialog to txt
void controller::on_CBrecepSetSaveDialog_clicked()
{
    if (ui->CBrecepSetSaveDialog->isChecked()){
        ui->BrecepSetSaveDialog->setEnabled(true);
        connect(this, &controller::SdialogSaveUpdate,
                this, &controller::slotAutoSaveDialog);
    }
    else{
        ui->BrecepSetSaveDialog->setEnabled(false);
        disconnect(this, &controller::SdialogSaveUpdate,
                   this, &controller::slotAutoSaveDialog);
    }
}

void controller::slotAutoSaveDialog(QString updateInfo)
{
    if (saveDialogFilePath != ""){
        QFile file(saveDialogFilePath);
        file.open(QFile::Append);
        file.write(updateInfo.toUtf8());
    }
}

void controller::on_BrecepSetSaveDialog_clicked()
{
    int fileAvailable = 0;
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
            fileAvailable = 1;
            QTextStream stream(&file);
            stream<<"THIS IS MAX DIALOG\n";
            stream<<dialogHistory;
            stream.flush();
            file.close();
        }
    }

    if (fileAvailable == 1)
        saveDialogFilePath = fileName;
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
    if (! SCmodel->m_s->isListening()){
    //start Listening
        emit STcpServerStartListening();
        connect(SCmodel, &model::SsocketListUpdate,
                this, &controller::slotUpdateClientList);
    }
    else{
    //close Listening
        emit STcpServerclose();
        disconnect(SCmodel, &model::SsocketListUpdate,
                   this, &controller::slotUpdateClientList);
    }
}

void controller::slotUpdateClientList(QList<QTcpSocket *> socketList)
{
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
}

//when clicking 连接button
void controller::on_BsendSetConnect_clicked()
{
    emit STcpClientConnectStartConnect();
    //Cmodel->m_c->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, 1024*1024);
}

void controller::slotTCPClientConnectSuccess()
{
    ui->Edialog->append("[" + QDateTime::currentDateTime().toString(timeFormat) + "] " +
                        tr("TCPClient State: CONNECT"));
    qDebug()<<Cmodel->m_c->socketDescriptor();
    ui->BsendSetBreak->setEnabled(true);
    ui->BsendSetConnect->setEnabled(false);
    if(ui->RBsendZoneClient->isChecked()){
        ui->LElocalHostAdd->setEnabled(false);
        ui->LElocalHostPort->setEnabled(false);
    }
}

void controller::slotTCPClientDisconnect()
{
    ui->Edialog->append("[" + QDateTime::currentDateTime().toString(timeFormat) + "] " +
                        tr("TCPClient State: DISCONNECT"));

    ui->BsendSetConnect->setEnabled(true);
    ui->BsendSetBreak->setEnabled(false);
    if(ui->RBsendZoneClient->isChecked()){
        ui->LElocalHostAdd->setEnabled(true);
        ui->LElocalHostPort->setEnabled(true);
    }
}

void controller::slotViewChangeTcpServerStartListening()
{
    ui->CBclientList->clear();
    ui->CBclientList->addItem(tr("未客户端连接"));
    ui->CBclientList->setEnabled(true);
    qDebug()<<"server state"<<SCmodel->m_s->isListening();
    if (SCmodel->m_s->isListening())
        TCPServerState = "LISTEN";
    else
        TCPServerState = "CLOSE";
    ui->BactivateListen->setText(tr("关闭服务器"));
    ui->CBtypePro->setEnabled(false);

    if(ui->RBsendZoneServer->isChecked())
        ui->LElocalHostPort->setEnabled(false);

    ui->Edialog->append("[" + QDateTime::currentDateTime().toString(timeFormat) +
                        "] " + "TCPServer State: " + TCPServerState);
}

void controller::slotViewChangeTcpServerClose()
{
    ui->CBclientList->clear();
    ui->CBclientList->addItem(tr("服务器未监听"));
    ui->CBclientList->setEnabled(false);
    qDebug()<<"server state"<<SCmodel->m_s->isListening();
    if (SCmodel->m_s->isListening())
        TCPServerState = "LISTEN";
    else
        TCPServerState = "CLOSE";
    ui->BactivateListen->setText(tr("启动监听"));
    ui->CBtypePro->setEnabled(true);

    if(ui->RBsendZoneServer->isChecked())
        ui->LElocalHostPort->setEnabled(true);

    ui->Edialog->append("[" + QDateTime::currentDateTime().toString(timeFormat) +
                        "] " + "TCPServer State: " + TCPServerState);
}

//disconnection
void controller::on_BsendSetBreak_clicked()
{
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
    //qDebug()<<"send message"<<tempInput;

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
                    emit SsendClientMsg(tempInput.toUtf8());
                    ui->Edialog->append("[" + QDateTime::currentDateTime().toString(timeFormat) + "] "
                                        + tr("Send by Client: ") + tempInput);
                    dialogHistory.append("[" + QDateTime::currentDateTime().toString(timeFormat) + "] "
                            + tr("Send by Client:") + tempInput.toUtf8().toHex() + "\n");
                    emit SdialogSaveUpdate("[" + QDateTime::currentDateTime().toString(timeFormat) + "] "
                                           + tr("Send by Client:") + tempInput.toUtf8().toHex() + "\n");
                }
                else{
                //adding head to text
                    QString temp = head + QString::number(tempInput.length())
                            + "|" + tempInput;
                    emit SsendClientMsg(temp.toUtf8());
                    ui->Edialog->append("[" + QDateTime::currentDateTime().toString(timeFormat) + "] "
                                        + tr("Send by Client: ") + tempInput);
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
                emit SsendServerMsg(tempInput.toUtf8(), ui->CBclientList->currentIndex());

                ui->Edialog->append("[" + QDateTime::currentDateTime().toString(timeFormat) + "] "
                                    + tr("Send by Server: ") + tempInput);
                dialogHistory.append("[" + QDateTime::currentDateTime().toString(timeFormat) + "] "
                        + tr("Send by Server:") +tempInput.toUtf8().toHex() + "\n");
                emit SdialogSaveUpdate("[" + QDateTime::currentDateTime().toString(timeFormat) + "] "
                                       + tr("Send by Server:") +tempInput.toUtf8().toHex() + "\n");
            }
            else
            //add head head int(size) text
            {
                QString temp = head + QString::number(tempInput.length())
                        + "|" + tempInput;
                emit SsendServerMsg(temp.toUtf8(), ui->CBclientList->currentIndex());

                ui->Edialog->append("[" + QDateTime::currentDateTime().toString(timeFormat) + "] "
                                    + tr("Send by Server: ") + tempInput);
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
    if(ui->CBopenFile->isChecked())
    //open file and send
    {
        qDebug()<<"send open file";
        emit SsendOpenFile(openFilePath);
    }
    else
    //send simple message
        sendMessage();
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

//input host and ip
void controller::on_LElocalHostAdd_textChanged(const QString &arg1)
{
    if(ui->CBtypePro->currentText() == "TCP")
    {
        if(ui->RBsendZoneClient->isChecked())
            Cmodel->ip = arg1;
    }
    else if(ui->CBtypePro->currentText() == "UDP")
        udpClient->ip = arg1;
}

void controller::on_LElocalHostPort_textChanged(const QString &arg1)
{
    if(ui->CBtypePro->currentText() == "TCP"){
        if (ui->RBsendZoneServer->isChecked())
            SCmodel->port = arg1.toInt();
        else if(ui->RBsendZoneClient->isChecked())
            Cmodel->port = arg1.toInt();
    }
    else if(ui->CBtypePro->currentText() == "UDP")
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
            ui->Edialog->append("[" + QDateTime::currentDateTime().toString(timeFormat) + "] " + tr("获得UDP传输：") + str);
        else
        //apear HEX
            ui->Edialog->append("[" + QDateTime::currentDateTime().toString(timeFormat) + "] " + tr("获得UDP传输：") + str.toLatin1().toHex());
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
                    ui->Edialog->append("[" + QDateTime::currentDateTime().toString(timeFormat) + "] " + tr("获得UDP传输：") + realMsg.toUtf8() + " " +
                                tr("信息长度") + QString::number(lengthOfget));
                else
                //apear in HEX
                    ui->Edialog->append("[" + QDateTime::currentDateTime().toString(timeFormat) + "] " + tr("获得UDP传输：") + realMsg.toUtf8().toHex() + " " +
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
            this,[=](QDateTime time){
        ui->Edialog->append("[" + QDateTime::currentDateTime().toString(timeFormat) + "] " +
                            tr("Going to repeat dialog starting at ") + '[' + time.toString(timeFormat) + ']');
    });
    connect(sendDialog, &sendByDialog::SrepeatComplete,
            this,[=](){
        ui->Edialog->append("[" + QDateTime::currentDateTime().toString(timeFormat) + "] " +
                            tr("Dialog read Complete"));
    });
    connect(sendDialog, &sendByDialog::SopenWrongDialog,
            this, &controller::repeatNotMyDialog, Qt::QueuedConnection);
    connect(sendDialog, &sendByDialog::SendTask,
            this, [=](){
        sendDialog->close();
        disconnect(sendDialog,&sendByDialog::sendInformation,
                   this, &controller::repeatDialog);
        disconnect(sendDialog, &sendByDialog::SopenWrongDialog,
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

//record message
void controller::on_PBrecordHistory_clicked()
{
    if (recordState == 0)
    {
    //start recording
        recordFilePath = QFileDialog::getOpenFileName();
        if (recordFilePath == "")
        {
            QMessageBox::warning(this, "打开文件", "选择的文件路径不能为空");
            return;
        }
        ui->PBrecordHistory->setText(tr("停止录制"));
        recordState = 1;
        ui->GBrecordSideSelect->setEnabled(false);
        if (ui->RBrecordSideSelectClient->isChecked())
        //record client message
            connect(Cmodel, &model_client::SgetServerMessage,
                    this,&controller::slotRecordMsg);
        else
            connect(SCmodel, &model::SgetClientMessage,
                    this, &controller::slotRecordMsg);
    }
    else
    {
    //stop recording
        ui->PBrecordHistory->setText(tr("开始录制"));
        recordState = 0;
        ui->GBrecordSideSelect->setEnabled(true);
        if (ui->RBrecordSideSelectClient->isChecked())
            disconnect(Cmodel, &model_client::SgetServerMessage,
                    this,&controller::slotRecordMsg);
        else
            disconnect(SCmodel, &model::SgetClientMessage,
                    this, &controller::slotRecordMsg);
    }
}

void controller::slotRecordMsg(QByteArray msg)
{
    QFile file(recordFilePath);
    file.open(QFile::Append);
    file.write(msg.toHex());
    file.close();
}

void controller::on_CBopenFile_toggled(bool checked)
{
    if(checked)
    {
        openFilePath = QFileDialog::getOpenFileName();
        if (openFilePath.isEmpty())
        {
            QMessageBox::warning(this, "打开文件", "选择的文件路径不能为空");
            return;
        }
        ui->EtextSend->setReadOnly(true);
        ui->EtextSend->setText(openFilePath);
        ui->GBsendSideSelect->setEnabled(false);
        ui->GBsendSetInputType->setEnabled(false);

        openFile = new sendFileInThread;
        openFileThread = new QThread;
        openFile->moveToThread(openFileThread);
        openFileThread->start();
        connect(this, &controller::SsendOpenFile,
                openFile, &sendFileInThread::sendFile);
        connect(openFile, &sendFileInThread::SreadTextLine,
                this, &controller::slotSendOpenFile);
        connect(openFile, &sendFileInThread::SfileSendSuccess,
                this, [=](){
            ui->EtextSend->setText(tr("文件读取完毕"));
        });
    }
    else
    {
        ui->EtextSend->clear();
        ui->EtextSend->setReadOnly(false);
        ui->GBsendSideSelect->setEnabled(true);
        ui->GBsendSetInputType->setEnabled(true);

        disconnect(this, &controller::SsendOpenFile,
                openFile, &sendFileInThread::sendFile);
        disconnect(openFile, &sendFileInThread::SreadTextLine,
                this, &controller::slotSendOpenFile);
    }
}

void controller::slotSendOpenFile(QByteArray line)
{
    ui->EtextSend->setText(QString(line));
    sendMessage();
}

void controller::on_RBsendZoneServer_toggled(bool checked)
{
    if(checked)
    {
        ui->Lip->setText(tr("本地主机地址"));
        ui->Lport->setText(tr("本地监听端口"));

        ui->LElocalHostAdd->setEnabled(false);
        ui->LElocalHostAdd->setText("HostAddress: ANY");

        ui->LElocalHostPort->setEnabled(true);
        ui->LElocalHostPort->setText(QString::number(SCmodel->port));

        ui->CBclientList->setEnabled(true);
        if(TCPServerState == "LISTEN")
            ui->LElocalHostPort->setEnabled(false);
    }
}

void controller::on_RBsendZoneClient_toggled(bool checked)
{
    if(checked)
    {
        ui->Lip->setText(tr("远程主机地址"));
        ui->Lport->setText(tr("远程主机端口"));

        ui->LElocalHostAdd->setEnabled(true);
        ui->LElocalHostAdd->setText(Cmodel->ip);

        ui->LElocalHostPort->setEnabled(true);
        ui->LElocalHostPort->setText(QString::number(Cmodel->port));

        ui->CBclientList->setEnabled(false);
        if (Cmodel->m_c->state() == QTcpSocket::ConnectedState)
        {
            ui->LElocalHostAdd->setEnabled(false);
            ui->LElocalHostPort->setEnabled(false);
        }
    }
}
